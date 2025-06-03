#include "transform.hpp"

glm::mat4 transform::get_model_matrix() {
	glm::mat4 scaleM = glm::scale(glm::mat4(1.0f), scale);
	glm::mat4 rotateM = glm::mat4_cast(rotation);
	glm::mat4 translateM = glm::translate(glm::mat4(1.0f), position);
	return translateM * rotateM * scaleM;
}
