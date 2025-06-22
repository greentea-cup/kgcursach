#ifndef SHADER_HPP
#define SHADER_HPP

#include <optional>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include "object.hpp"

#define LIGHTS_COUNT 16
struct point_lights {
	glm::vec3 positions_w[LIGHTS_COUNT] = {};
	glm::vec3 colors[LIGHTS_COUNT] = {};
	float powers[LIGHTS_COUNT] = {0.};
	void calc_positions_mv(glm::mat4 view);
	glm::vec3 positions_mv[LIGHTS_COUNT] = {};
	bool add(glm::vec3 position_w, glm::vec3 color, float power);
	size_t len = 0;
};

struct solid_shader_program {
	GLuint program;
	// uniforms
	GLint model;            // mat4
	GLint view;             // mat4
	GLint projection;       // mat4
	GLint normal_matrix_mv; // mat3
	GLint diffuse_color;    // vec4
	GLint diffuse_map;      // sampler2D
	GLint has_diffuse_map;  // bool
	GLint normal_map;       // sampler2D
	GLint has_normal_map;   // bool
	GLint lights_pos_mv;    // vec3[LIGHTS_COUNT]
	GLint lights_color;     // vec3[LIGHTS_COUNT]
	GLint lights_power;     // float[LIGHTS_COUNT]

	static std::optional<solid_shader_program> create(char const *vertex_path, char const *fragment_path);
	void set_view(glm::mat4 const &view_mat);
	void set_projection(glm::mat4 const &projection_mat);
	void set_model(glm::mat4 const &model_mat);
	void set_normal_mv(glm::mat3 const &normal_mat_mv);
	void set_material(material const &material);
	void set_lights(point_lights const lights);
private:
	solid_shader_program() {}
};

#endif
