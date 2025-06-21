#include "transform.hpp"
#include <glm/gtc/matrix_inverse.hpp>

void transform::recalc_matricies() {
	glm::mat4 scaleM = glm::scale(glm::mat4(1.0f), _scale);
	glm::mat4 rotateM = glm::mat4_cast(_rotation);
	glm::mat4 translateM = glm::translate(glm::mat4(1.0f), _position);
	_model_matrix = translateM * rotateM * scaleM;
}
