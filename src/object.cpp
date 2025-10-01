// vim: set fdm=indent :
#include <GL/glew.h>
#include <SDL2/SDL_log.h>
#include "object.hpp"
#include "registry.hpp"

bool object::has_mesh() {
	auto it = meshes.find(mesh_name);
	return it != meshes.end();
}
struct mesh &object::mesh() { return meshes.at(mesh_name); }

bool mesh::has_material() {
	auto it = materials.find(material_name);
	return it != materials.end();
}

struct material &mesh::material() { return materials.at(material_name); }

void mesh::gen_tbn() {
	// https://www.opengl-tutorial.org/intermediate-tutorials/tutorial-13-normal-mapping
	tangents.clear();
	size_t s = vertices.size();
	tangents.reserve(s);
	for (size_t i = 0; i < s; i += 3) {
		glm::vec3 delta_pos1 = vertices[i+1] - vertices[i+0];
		glm::vec3 delta_pos2 = vertices[i+2] - vertices[i+0];
		glm::vec2 delta_uv1 = uvs[i+1] - uvs[i+0];
		glm::vec2 delta_uv2 = uvs[i+2] - uvs[i+0];
		float r = 1.f / (delta_uv1.x * delta_uv2.y - delta_uv1.y * delta_uv2.x);
		glm::vec3 tangent = r * (delta_pos1 * delta_uv2.y - delta_pos2 * delta_uv1.y);
		tangents.push_back(tangent);
		tangents.push_back(tangent);
		tangents.push_back(tangent);
	}
}

bool mesh::prepare_to_drawing() {
	// SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Preparing mesh '%s'\n", this->name.c_str());
	if (vao == 0) {
		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vertices_buf);
		glGenBuffers(1, &uvs_buf);
		glGenBuffers(1, &normals_buf);
		glGenBuffers(1, &tangents_buf);
	}
	if (vao == 0 || vertices_buf == 0 || uvs_buf == 0 || normals_buf == 0 || tangents_buf == 0) {
		if (vao != 0) glDeleteVertexArrays(1, &vao);
		if (vertices_buf != 0) glDeleteBuffers(1, &vertices_buf);
		if (uvs_buf != 0) glDeleteBuffers(1, &uvs_buf);
		if (normals_buf != 0) glDeleteBuffers(1, &normals_buf);
		if (tangents_buf != 0) glDeleteBuffers(1, &tangents_buf);
		vao = vertices_buf = uvs_buf = normals_buf = tangents_buf = 0;
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "mesh::prepare_to_drawing: Failed to initialize opengl buffers\n");
		return false;
	}
	if (tangents.empty()) gen_tbn();
	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vertices_buf);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices[0]), vertices.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, uvs_buf);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(uvs[0]), uvs.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, false, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, normals_buf);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(normals[0]), normals.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, false, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, tangents_buf);
	glBufferData(GL_ARRAY_BUFFER, tangents.size() * sizeof(tangents[0]), tangents.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, false, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	// SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Finished preparing mesh '%s'\n", this->name.c_str());
	return true;
}
