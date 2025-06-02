// vim: set fdm=indent :
#ifndef LOADER_HPP
#define LOADER_HPP
#include <string>
#include <optional>
#include <vector>
#include <glm/glm.hpp>
#include <GL/gl.h>

/**
Loads vertex and fragment shaders from vert_path and frag_path respectively,
Creates program and puts created program's id into out_program and returns true on success.
Returns false if out_pogram is nullptr.
On error returns false.
*/
bool load_shader_program(char const *vert_path, char const *frag_path, char const *progname, GLuint *out_program);

bool load_texture(char const *filepath, GLuint *out_texture);

struct mesh {
	std::string name;
	std::string material_lib;
	std::string material_name;
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;

	static std::optional<mesh> from_obj_file(char const *filepath);

	mesh(mesh &&o) :
		vertices(std::move(o.vertices)),
		uvs(std::move(o.uvs)),
		normals(std::move(o.normals)) {}

private:
	mesh() {}
};

struct material {
	std::string name;
	glm::vec3 ambient = glm::vec3(0);
	glm::vec3 diffuse = glm::vec3(0);
	glm::vec3 specular = glm::vec3(0);
	float specular_highlight = 0;
	float transparency = 1; // 0 = fully transparent, 1 = fully opaque
	std::optional<std::string> ambient_texture = {};
	std::optional<std::string> diffuse_texture = {};
	std::optional<std::string> specular_texture = {};
	std::optional<std::string> specular_highlight_texture = {};

	// allows only one material per file
	static std::optional<material> from_mtl_file(char const *filepath);

	material(material &&o) :
		name(std::move(o.name)),
		ambient(std::move(o.ambient)),
		diffuse(std::move(o.diffuse)),
		specular(std::move(o.specular)),
		specular_highlight(std::move(o.specular_highlight)),
		transparency(std::move(o.transparency)),
		ambient_texture(std::move(o.ambient_texture)),
		diffuse_texture(std::move(o.diffuse_texture)),
		specular_texture(std::move(o.specular_texture)),
		specular_highlight_texture(std::move(o.specular_highlight_texture))
	{}

private:
	material() {}
};

#endif
