// vim: set fdm=indent :
#include <fstream>
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <stb_image.h>
#include "util.hpp"
#include "loader.hpp"
#include "registry.hpp"
#include "object.hpp"

bool load_shader_program(char const *vert_path, char const *frag_path, char const *progname, GLuint *out_program) {
	if (out_program == nullptr) return false;
	GLuint vert = glCreateShader(GL_VERTEX_SHADER);
	GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);

	std::string vert_src = read_file(vert_path);
	std::string frag_src = read_file(frag_path);
	char const *srcs[2] = {vert_src.c_str(), frag_src.c_str()};
	glShaderSource(vert, 1, &srcs[0], NULL);
	glShaderSource(frag, 1, &srcs[1], NULL);
	
	glCompileShader(vert);
	glCompileShader(frag);

	GLint vert_compiled, frag_compiled;
	glGetShaderiv(vert, GL_COMPILE_STATUS, &vert_compiled);
	glGetShaderiv(frag, GL_COMPILE_STATUS, &frag_compiled);
	if (!(vert_compiled && frag_compiled)) {
		char infolog[1024];
		if (!vert_compiled) {
			glGetShaderInfoLog(vert, sizeof(infolog), NULL, infolog);
			SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Failed to compile vertex shader for program %s:\n%s", progname, infolog);
		}
		if (!frag_compiled) {
			glGetShaderInfoLog(frag, sizeof(infolog), NULL, infolog);
			SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Failed to compile fragment shader for program %s:\n%s", progname, infolog);
		}
		glDeleteShader(vert);
		glDeleteShader(frag);
		return false;
	}

	GLuint prog = glCreateProgram();
	glAttachShader(prog, vert);
	glAttachShader(prog, frag);
	glLinkProgram(prog);
	glDeleteShader(vert);
	glDeleteShader(frag);
	GLint prog_linked;
	glGetProgramiv(prog, GL_LINK_STATUS, &prog_linked);
	if (!prog_linked) {
		char infolog[1024];
		glGetProgramInfoLog(prog, sizeof(infolog), NULL, infolog);
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Failed to link program %s:\n%s", progname, infolog);
		return false;
	}

	*out_program = prog;
	return true;
}

bool load_texture(char const *filepath, GLuint *out_texture) {
	int width, height, channels;
	stbi_set_flip_vertically_on_load(1);
	unsigned char *data = stbi_load(filepath, &width, &height, &channels, 4);
	if (data == NULL) { return false; }
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
	*out_texture = tex;
	return true;
}

void register_mesh(char const *object_resource) {
	if (object_resource == nullptr) return;
	char path[256];
	snprintf(path, sizeof(path), "data/objects/%s.obj", object_resource);
	SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Loading mesh from %s\n", path);
	std::ifstream fp(path);
	if (!fp.is_open()) return;
	mesh res;
	std::vector<glm::vec3> vertices, face_vertices;
	std::vector<glm::vec2> uvs, face_uvs;
	std::vector<glm::vec3> normals, face_normals;
	char line[1024];
	std::vector<size_t> tmp;
	tmp.reserve(64);
	while (1) {
		fp.getline(line, sizeof(line));
		if (fp.eof()) break;
		if (line[0] == '#') /* comment */ continue;
		else if (!strncmp("mtllib ", line, 7)) {
			res.material_lib = std::string(line + 7);
			register_materials(line + 7);
		}
		else if (!strncmp("o ", line, 2)) {
			res.name = std::string(line + 2);
			SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Loading object %s\n", res.name.c_str());
		}
		else if (!strncmp("v ", line, 2)) {
			// only x y z coords
			glm::vec3 v;
			sscanf(line, "v %f %f %f", &v.x, &v.y, &v.z);
			vertices.push_back(v);
		}
		else if (!strncmp("vt ", line, 3)) {
			// only u v coords
			glm::vec2 vt;
			sscanf(line, "vt %f %f", &vt.x, &vt.y);
			uvs.push_back(vt);
		}
		else if (!strncmp("vn ", line, 3)) {
			// only x y z coords
			glm::vec3 vn;
			sscanf(line, "vn %f %f %f", &vn.x, &vn.y, &vn.z);
			normals.push_back(vn);
		}
		else if (!strncmp("usemtl ", line, 7)) {
			// corresponding material shuld alredy be loaded
			// and stored in registry
			res.material_name = std::string(line + 7);
			if (materials.find(res.material_name) == materials.end()) {
				SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Material %s not found\n", line + 7);
			}
		}
		else if (!strncmp("f ", line, 2)) {
			SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "face: '%s'\n", line);
			// only 3 or 4 vertices allowed
			// partially-filled data not allowed
			// triangle is 0,1,2
			// quad is 0,1,2 + 0,2,3 (triangles)
			tmp.clear();
			tmp.reserve(12);
			ssize_t len = (ssize_t)strlen(line);
			char const *l = line + 2;
			while (l - line < len) {
				uint x = 0; while (l[x] != ' ' && l[x] != '\n' && l[x] != '\0') x++;
				SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "entry len %u (%.*s)\n", x, x, l);
				if (x == 0) break;
				uint y1 = 0; while (l[y1] != '/') y1++;
				uint y2 = y1 + 1; while (l[y2] != '/') y2++;
				SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "entry splits at %u %u\n", y1, y2);
				size_t v = 1, vt = 1, vn = 0;
				(void)sscanf(l, "%zu", &v);
				if (y1 > 0) (void)sscanf(l + y1 + 1, "%zu", &vt);
				if (y2 > y1+1) (void)sscanf(l + y2 + 1, "%zu", &vn);
				SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "entry values %zu %zu %zu\n", v, vt, vn);
				tmp.push_back(v-1);
				tmp.push_back(vt-1);
				tmp.push_back(vn-1);
				l += x + 1;
			}
			switch (tmp.size()) {
				default: break;
				case 9: /* triangle 0,1,2 */ {
					face_vertices.push_back(vertices[tmp[0]]);
					face_vertices.push_back(vertices[tmp[3]]);
					face_vertices.push_back(vertices[tmp[6]]);

					face_uvs.push_back(uvs[tmp[1]]);
					face_uvs.push_back(uvs[tmp[4]]);
					face_uvs.push_back(uvs[tmp[7]]);

					face_normals.push_back(normals[tmp[2]]);
					face_normals.push_back(normals[tmp[5]]);
					face_normals.push_back(normals[tmp[8]]);
				} break;
				case 12: /* quad 0,1,2 + 0,2,3 */ {
					face_vertices.push_back(vertices[tmp[0]]);
					face_vertices.push_back(vertices[tmp[3]]);
					face_vertices.push_back(vertices[tmp[6]]);
					face_vertices.push_back(vertices[tmp[0]]);
					face_vertices.push_back(vertices[tmp[6]]);
					face_vertices.push_back(vertices[tmp[9]]);

					face_uvs.push_back(uvs[tmp[ 1]]);
					face_uvs.push_back(uvs[tmp[ 4]]);
					face_uvs.push_back(uvs[tmp[ 7]]);
					face_uvs.push_back(uvs[tmp[ 1]]);
					face_uvs.push_back(uvs[tmp[ 7]]);
					face_uvs.push_back(uvs[tmp[10]]);

					face_normals.push_back(normals[tmp[ 2]]);
					face_normals.push_back(normals[tmp[ 5]]);
					face_normals.push_back(normals[tmp[ 8]]);
					face_normals.push_back(normals[tmp[ 2]]);
					face_normals.push_back(normals[tmp[ 8]]);
					face_normals.push_back(normals[tmp[11]]);
				} break;
			}
		}
	}
	res.vertices = std::move(face_vertices);
	res.uvs = std::move(face_uvs);
	res.normals = std::move(face_normals);
	fp.close();
	std::string objname(res.name);
	auto it = meshes.find(objname);
	// delete mesh in case it is already loaded
	// probably better do this than just override
	if (it != meshes.end()) meshes.erase(it);
	meshes.insert(it, {objname, std::move(res)});
}

void register_materials(char const *material_lib_resource) {
	if (material_lib_resource == nullptr) return;
	char filepath[256];
	// .mtl is already included in obj file
	snprintf(filepath, sizeof(filepath), "data/materials/%s", material_lib_resource);
	SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Loading material from %s\n", filepath);
	std::ifstream fp(filepath);
	if (!fp.is_open()) return;
	material res;
	char line[1024];
	bool first_material = true;
	while (1) {
		fp.getline(line, sizeof(line));
		if (fp.eof()) break;
		if (line[0] == '#') /* comment */ continue;
		else if (!strncmp("newmtl ", line, 7)) {
			SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Started filling material %s\n", line + 7);
			if (first_material) {
				first_material = false;
				res.name = std::string(line + 7);
			}
			else {
				std::string name(res.name);
				materials.insert({name, std::move(res)});
				res = material{}; // start with new material
				res.name = std::string(line + 7);
			}
		}
		else if (!strncmp("Ka ", line, 3)) {
			glm::vec3 Ka;
			sscanf(line, "Ka %f %f %f", &Ka.x, &Ka.y, &Ka.z);
			res.ambient = Ka;
		}
		else if (!strncmp("Kd ", line, 3)) {
			glm::vec3 Kd;
			sscanf(line, "Kd %f %f %f", &Kd.x, &Kd.y, &Kd.z);
			res.diffuse = Kd;
		}
		else if (!strncmp("Ks ", line, 3)) {
			glm::vec3 Ks;
			sscanf(line, "Ks %f %f %f", &Ks.x, &Ks.y, &Ks.z);
			res.specular = Ks;
		}
		else if (!strncmp("Ns ", line, 3)) {
			float Ns;
			sscanf(line, "Ns %f", &Ns);
			res.specular_highlight = Ns;
		}
		else if (!strncmp("d ", line, 2)) {
			float d;
			sscanf(line, "d %f", &d);
			res.alpha = d;
			if (d < 1.) res.is_transparent = true;
		}
		else if (!strncmp("Tr ", line, 3)) {
			float Tr;
			sscanf(line, "Tr %f", &Tr);
			res.alpha = 1 - Tr;
			if (Tr > 0.) res.is_transparent = true;
		}
		else if (!strncmp("map_Ka ", line, 7)) {
			res.ambient_texpath = std::string(line + 7);
			auto it = textures.find(res.ambient_texpath.value());
			if (it == textures.end()) {
				res.ambient_texture = register_texture(line + 7);
			}
		}
		else if (!strncmp("map_Kd ", line, 7)) {
			res.diffuse_texpath = std::string(line + 7);
			auto it = textures.find(res.diffuse_texpath.value());
			if (it == textures.end()) {
				res.diffuse_texture = register_texture(line + 7);
			}
		}
		else if (!strncmp("map_Ks ", line, 7)) {
			res.specular_texpath = std::string(line + 7);
			auto it = textures.find(res.specular_texpath.value());
			if (it == textures.end()) {
				res.specular_highlight_texture = register_texture(line + 7);
			}
		}
		else if (!strncmp("map_Ns ", line, 7)) {
			res.specular_highlight_texpath = std::string(line + 7);
			auto it = textures.find(res.specular_highlight_texpath.value());
			if (it == textures.end()) {
				res.specular_highlight_texture = register_texture(line + 7);
			}
		}
		else if (!strncmp("map_Bump ", line, 9) || !strncmp("map_bump ", line, 9))  {
			res.normal_texpath = std::string(line + 9);
			auto it = textures.find(res.normal_texpath.value());
			if (it == textures.end()) {
				res.normal_texture = register_texture(line + 9);
			}
		}
		else if (!strncmp("bump ", line, 5) || !strncmp("Bump ", line, 5)) {
			res.normal_texpath = std::string(line + 5);
			auto it = textures.find(res.normal_texpath.value());
			if (it == textures.end()) {
				res.normal_texture = register_texture(line + 5);
			}
		}
	}
	// Add last material
	std::string name(res.name);
	materials.insert({name, std::move(res)});
	fp.close();
}
std::optional<GLuint> register_texture(char const *texture_name) {
	char filepath[256];
	// mtllib has extension too, obviously
	snprintf(filepath, sizeof(filepath), "data/textures/%s", texture_name);
	SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Loading texture from %s\n", filepath);
	GLuint tex;
	if (load_texture(filepath, &tex)) {
		auto it = textures.find(texture_name);
		// delete old texture in case
		if (it != textures.end()) {
			glDeleteTextures(1, &it->second);
		}
		textures.insert(it, {texture_name, tex});
		SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Successfully loaded texture '%s'\n", texture_name);
		return tex;
	}
	else {
		SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Error while loading texture\n");
		return {};
	}
}

