// vim: set fdm=indent :
#include <GL/glew.h>
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

bool mesh::prepare_to_drawing() {
	if (vao == 0) {
		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vertices_buf);
		glGenBuffers(1, &uvs_buf);
		glGenBuffers(1, &normals_buf);
	}
	if (vao == 0 || vertices_buf == 0 || uvs_buf == 0 || normals_buf == 0) {
		if (vao != 0) glDeleteVertexArrays(1, &vao);
		if (vertices_buf != 0) glDeleteBuffers(1, &vertices_buf);
		if (uvs_buf != 0) glDeleteBuffers(1, &uvs_buf);
		if (normals_buf != 0) glDeleteBuffers(1, &normals_buf);
		vao = vertices_buf = uvs_buf = normals_buf = 0;
		return false;
	}
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

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	return true;
}
