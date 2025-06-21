#ifndef TRANSFORM_HPP
#define TRANSFORM_HPP

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

struct transform {
	void set_position(glm::vec3 position) {
		_position = position;
		recalc_matricies();
	}
	void set_scale(glm::vec3 scale) {
		_scale = scale;
		recalc_matricies();
	}
	void set_rotation(glm::quat rotation) {
		_rotation = rotation;
		recalc_matricies();
	}

	glm::vec3 position() { return _position; }
	glm::vec3 scale() { return _scale; }
	glm::quat rotation() { return _rotation; }

	glm::mat4 model_matrix() { return _model_matrix; }

private:
	void recalc_matricies();
	glm::vec3 _position = glm::vec3(0, 0, 0);
	glm::vec3 _scale = glm::vec3(1, 1, 1);
	glm::quat _rotation = glm::quat(glm::vec3(0, 0, 0));
	glm::mat4 _model_matrix = {};
};

#endif
