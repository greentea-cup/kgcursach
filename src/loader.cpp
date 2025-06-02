// vim: set fdm=indent :
#include <fstream>
#include <GL/glew.h>
#include <SDL.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <stb_image.h>
#include "util.hpp"
#include "loader.hpp"

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
	unsigned char *data = stbi_load(filepath, &width, &height, &channels, 4);
	if (data == NULL) return false;
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	*out_texture = tex;
	return true;
}

std::optional<mesh> mesh::from_obj_file(char const *filepath) {
	if (filepath == nullptr) return {};
	SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Loading mesh from %s\n", filepath);
	std::ifstream fp(filepath);
	if (!fp.is_open()) return {};
	mesh res;
	std::vector<glm::vec3> vertices, face_vertices;
	std::vector<glm::vec2> uvs, face_uvs;
	std::vector<glm::vec3> normals, face_normals;
	// push index 0 params because indexing in faces starts from 1
	// and i don't bother subtracting
	vertices.push_back(glm::vec3(0, 0, 0));
	uvs.push_back(glm::vec2(0, 0));
	normals.push_back(glm::vec3(0, 0, 0));
	char line[1024];
	std::vector<size_t> tmp;
	tmp.reserve(64);
	while (1) {
		fp.getline(line, sizeof(line));
		if (fp.eof()) break;
		if (line[0] == '#') /* comment */ continue;
		else if (!strncmp("mtllib ", line, 7)) {
			char path[1024];
			snprintf(path, sizeof(path), "data/materials/%s", line + 7);
			res.material_lib = std::string(path);
		}
		else if (!strncmp("o ", line, 2)) {
			res.name = std::string(line + 2);
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
			sscanf(line, "v %f %f %f", &vn.x, &vn.y, &vn.z);
			vertices.push_back(vn);
		}
		else if (!strncmp("usemtl ", line, 7)) {
			res.material_name = std::string(line + 7);
		}
		else if (!strncmp("f ", line, 2)) {
			SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "face: '%s'\n", line);
			// only 3 or 4 vertices allowed
			// partially-filled data not allowed
			// triangle is 0,1,2
			// quad is 0,1,2 + 0,2,3 (triangles)
			tmp.clear();
			ssize_t len = (ssize_t)strlen(line);
			char const *l = line + 2;
			while (l - line < len) {
				uint x = 0; while (l[x] != ' ' && l[x] != '\n' && l[x] != '\0') x++;
				SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "entry len %u (%.*s)\n", x, x, l);
				if (x == 0) break;
				uint y1 = 0; while (l[y1] != '/') y1++;
				uint y2 = y1 + 1; while (l[y2] != '/') y2++;
				SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "entry splits at %u %u\n", y1, y2);
				size_t v = 0, vt = 0, vn = 0;
				(void)sscanf(l, "%zu", &v);
				if (y1 > 0) (void)sscanf(l + y1 + 1, "%zu", &vt);
				if (y2 > y1+1) (void)sscanf(l + y2 + 1, "%zu", &vn);
				SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "entry values %zu %zu %zu\n", v, vt, vn);
				tmp.push_back(v);
				tmp.push_back(vt);
				tmp.push_back(vn);
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
					SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "verts %zu %zu %zu %zu\n", tmp[0], tmp[3], tmp[6], tmp[9]);
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
	return res;
}

std::optional<material> material::from_mtl_file(char const *filepath) {
	if (filepath == nullptr) return {};
	SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Loading material from %s\n", filepath);
	std::ifstream fp(filepath);
	if (!fp.is_open()) return {};
	material res;
	char line[1024];
	while (1) {
		fp.getline(line, sizeof(line));
		if (fp.eof()) break;
		if (line[0] == '#') /* comment */ continue;
		else if (!strncmp("newmtl ", line, 7)) {
			res.name = std::string(line + 7);
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
			res.transparency = d;
		}
		else if (!strncmp("Tr ", line, 3)) {
			float Tr;
			sscanf(line, "Tr %f", &Tr);
			res.transparency = 1 - Tr;
		}
		else if (!strncmp("map_Ka ", line, 7)) {
			res.ambient_texture = std::string(line + 7);
		}
		else if (!strncmp("map_Kd ", line, 7)) {
			res.diffuse_texture = std::string(line + 7);
		}
		else if (!strncmp("map_Ks ", line, 7)) {
			res.specular_texture = std::string(line + 7);
		}
		else if (!strncmp("map_Ns ", line, 7)) {
			res.specular_highlight_texture = std::string(line + 7);
		}
	}
	fp.close();
	return res;
}
