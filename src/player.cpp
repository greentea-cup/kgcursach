#include "player.hpp"

float player::pitch_min = glm::radians(-89.5f);
float player::pitch_max = glm::radians(+89.5f);
float player::mouse_sensitivity = glm::pi<float>() * (1/180.f) * 0.1f; // degrees(1.0f) is 1 radian in degrees

glm::vec3 player::front() {
	glm::vec3 dir;
	dir.x = glm::cos(rotation.x) * glm::cos(rotation.y);
	dir.y = glm::sin(rotation.y);
	dir.z = glm::sin(rotation.x) * glm::cos(rotation.y);
	return glm::normalize(dir);
}

glm::vec3 player::front_xz() {
	glm::vec3 dir;
	dir.x = glm::cos(rotation.x);
	dir.y = 0;
	dir.z = glm::sin(rotation.x);
	return glm::normalize(dir);
}

void player::rotate_camera(float dx, float dy) {
	rotation.x = fmod(rotation.x + dx * mouse_sensitivity, 2 * glm::pi<float>());
	rotation.y = glm::clamp(rotation.y + dy * mouse_sensitivity, pitch_min, pitch_max);
}

void player::move(float delta_time) {
	if (movement.x == 0 && movement.y == 0 && movement.z == 0) return;
	glm::vec3 direction = glm::vec3(0);
	glm::vec3 f = front_xz(), u = up();
	glm::vec3 r = glm::normalize(glm::cross(f, u));
	if (movement.x != 0) direction += (movement.x * 1.f) * f;
	if (movement.y != 0) direction += (movement.y * 1.f) * r;
	if (movement.z != 0) direction += (movement.z * 1.f) * u;
	position += glm::normalize(direction) * delta_time;
}
