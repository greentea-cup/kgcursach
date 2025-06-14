#ifndef PLAYER_HPP
#define PLAYER_HPP
#include <glm/glm.hpp>
#include <glm/ext.hpp>

struct player {
	static float pitch_min; 
	static float pitch_max;
	static float mouse_sensitivity;

	glm::vec3 position = glm::vec3(-2, 2, 0);
	// x = yaw (rad), y = pitch (rad)
	glm::vec2 rotation = glm::vec2(0, 0);
	glm::ivec3 movement = glm::ivec3(0, 0, 0);;
	float speed = 2.5f;

	glm::vec3 front();
	glm::vec3 front_xz();
	glm::vec3 up() { return glm::vec3(0, 1, 0); }
	void rotate_camera(float dx, float dy);
	void move(float delta_time);
};
#endif
