#ifndef OBJECT_HPP
#define OBJECT_HPP
#include <string>
#include <optional>
#include <vector>
#include <GL/gl.h>
#include "transform.hpp"

struct mesh {
	std::string name;
	std::string material_lib;
	std::string material_name;
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec3> tangents = {};
	size_t size() { return vertices.size() * 3; /* 3 floats in each glm::vec3 */ };
	bool has_material();
	/**
	TODO: возвращать какой-нибуть материал-заглушку.
	*/
	struct material &material();
	GLuint vao = 0, vertices_buf = 0, uvs_buf = 0, normals_buf = 0, tangents_buf = 0;
	bool prepare_to_drawing();
	void gen_tbn();
};

struct material {
	std::string name;
	glm::vec3 ambient = glm::vec3(0);
	glm::vec3 diffuse = glm::vec3(0);
	glm::vec3 specular = glm::vec3(0);
	float specular_highlight = 0;
	float alpha = 1; // 0 = fully transparent, 1 = fully opaque
	std::optional<std::string> ambient_texpath = {};
	std::optional<std::string> diffuse_texpath = {};
	std::optional<std::string> specular_texpath = {};
	std::optional<std::string> specular_highlight_texpath = {};
	std::optional<std::string> normal_texpath = {};
	bool is_transparent = 0;
	std::optional<GLuint> ambient_texture = {};
	std::optional<GLuint> diffuse_texture = {};
	std::optional<GLuint> specular_texture = {};
	std::optional<GLuint> specular_highlight_texture = {};
	std::optional<GLuint> normal_texture = {};
};

struct object {
	std::string mesh_name;
	struct transform transform = {};
	bool has_mesh();
	/*
	Форматирует диск, если не находит нужный меш. Но это не точно.
	Если объект создаётся из меша, которого по какой-либо причине нет
	в каталлоге (registry: meshes) - что-то явно идёт не по плану.
	*/
	struct mesh &mesh();
};

#endif
