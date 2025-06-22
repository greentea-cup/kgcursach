#include "shader.hpp"
#include <SDL2/SDL_log.h>
#include <glm/ext.hpp>
#include "loader.hpp"
#include "registry.hpp"

std::optional<solid_shader_program> solid_shader_program::create(const char *vertex_path, const char *fragment_path) {
	GLuint program;
	if (!load_shader_program(vertex_path, fragment_path, "solid", &program)) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Cannot load solid shader program\n");
		return {};
	}
	solid_shader_program res;
	res.program = program;
	res.model            = glGetUniformLocation(program, "model");
	res.view             = glGetUniformLocation(program, "view");
	res.projection       = glGetUniformLocation(program, "projection");
	res.normal_matrix_mv = glGetUniformLocation(program, "normal_matrix_mv");
	res.diffuse_color    = glGetUniformLocation(program, "diffuse_color");
	res.diffuse_map      = glGetUniformLocation(program, "diffuse_map");
	res.has_diffuse_map  = glGetUniformLocation(program, "has_diffuse_map");
	res.normal_map       = glGetUniformLocation(program, "normal_map");
	res.has_normal_map   = glGetUniformLocation(program, "has_normal_map");
	res.lights_pos_mv    = glGetUniformLocation(program, "lights_pos_mv");
	res.lights_color     = glGetUniformLocation(program, "lights_color");
	res.lights_power     = glGetUniformLocation(program, "lights_power");
	return res;
}

void point_lights::calc_positions_mv(glm::mat4 view) {
	for (size_t i = 0; i < len && i < LIGHTS_COUNT; i++) {
		positions_mv[i] = glm::vec3(view * glm::vec4(positions_w[i], 1.0));
	}
}

bool point_lights::add(glm::vec3 position_w, glm::vec3 color, float power) {
	if (len >= LIGHTS_COUNT-1) return false;
	positions_w[len] = position_w;
	colors[len] = color;
	powers[len] = power;
	len++;
	return true;
}

void solid_shader_program::set_lights(point_lights const lights) {
	glUniform3fv(lights_pos_mv, LIGHTS_COUNT, (GLfloat const *)(lights.positions_mv));
	glUniform3fv(lights_color, LIGHTS_COUNT, (GLfloat const *)(lights.colors));
	glUniform1fv(lights_power, LIGHTS_COUNT, (GLfloat const *)(lights.powers));
}

void solid_shader_program::set_view(glm::mat4 const &view_mat) {
	glUniformMatrix4fv(view, 1, false, glm::value_ptr(view_mat));
}
void solid_shader_program::set_projection(glm::mat4 const &projection_mat) {
	glUniformMatrix4fv(projection, 1, false, glm::value_ptr(projection_mat));
}
void solid_shader_program::set_model(glm::mat4 const &model_mat) {
	glUniformMatrix4fv(model, 1, false, glm::value_ptr(model_mat));
}
void solid_shader_program::set_normal_mv(glm::mat3 const &normal_mat_mv) {
	glUniformMatrix3fv(normal_matrix_mv, 1, false, glm::value_ptr(normal_mat_mv));
}
void solid_shader_program::set_material(const material &material) {
	glUniform3fv(diffuse_color, 1, glm::value_ptr(material.diffuse));
	if (material.diffuse_texpath.has_value()) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textures.at(material.diffuse_texpath.value()));
		glUniform1i(diffuse_map, 0);
		glUniform1i(has_diffuse_map, 1);
	}
	else {
		glUniform1i(has_diffuse_map, 0);
	}
	if (material.normal_texpath.has_value()) {
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, textures.at(material.normal_texpath.value()));
		glUniform1i(normal_map, 1);
		glUniform1i(has_normal_map, 1);
	}
	else {
		glUniform1i(has_normal_map, 0);
	}
}
