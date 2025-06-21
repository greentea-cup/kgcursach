// vim: set fdm=indent :
#include <fstream>
#include <string>
#include <sstream>
#include <optional>
#include <vector>
#include <GL/glew.h>
#include <SDL.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include "loader.hpp"
#include "registry.hpp"
#include "player.hpp"
#include "transform.hpp"

void mainloop(SDL_Window *window) {
	register_mesh("dice");
	register_mesh("box");
	register_mesh("dungeon_wall");
	
	// load scene
	object objects[] = {
		{"Box"}, {"Wall"}, {"Wall"}, {"Wall"}, {"Wall"}, {"Wall"}, {"Dice"}, {"Dice"}, {"Dice"}, {"Dice"}, {"Dice"}, {"Dice"}
	};
	size_t objects_len = sizeof(objects) / sizeof(*objects);
	// objects[0] == light position
	objects[0].transform.set_scale(glm::vec3(0.1));
	objects[1].transform.set_position(glm::vec3(0, -1, 0));
	objects[1].transform.set_scale(glm::vec3(10));
	objects[2].transform.set_position(glm::vec3(0, 0, -5));
	objects[2].transform.set_rotation(glm::quat(glm::vec3(glm::radians(90.0f), 0, 0)));
	objects[3].transform.set_position(glm::vec3(0, 0, 5));
	objects[3].transform.set_rotation(glm::quat(glm::vec3(glm::radians(90.0f), glm::radians(180.0f), 0)));
	objects[4].transform.set_position(glm::vec3(-5, 0, 0));
	objects[4].transform.set_rotation(glm::quat(glm::vec3(glm::radians(90.0f), glm::radians(90.0f), 0)));
	objects[5].transform.set_position(glm::vec3(5, 0, 0));
	objects[5].transform.set_rotation(glm::quat(glm::vec3(glm::radians(90.0f), glm::radians(-90.0f), 0)));
	objects[6].transform.set_scale(glm::vec3(0.1));
	objects[6].transform.set_position(glm::vec3(0, 0.9, 0));
	objects[7].transform.set_scale(glm::vec3(0.1));
	objects[7].transform.set_position(glm::vec3(0, 0.7, 0));
	objects[8].transform.set_scale(glm::vec3(0.1));
	objects[8].transform.set_position(glm::vec3(0, 0.5, 0));
	objects[9].transform.set_position(glm::vec3(0, 0, 0));
	objects[10].transform.set_position(glm::vec3(2, 0, 0));
	objects[11].transform.set_position(glm::vec3(2, 0, 2));
	for (size_t i = 0; i < objects_len; i++) {
		objects[i].mesh().prepare_to_drawing();
	}
	for (size_t i = 0; i < objects_len; i++) {
		object &o = objects[i];
		if (!o.has_mesh()) continue;
		mesh &m = o.mesh();
		if (m.has_material() && m.material().is_transparent) {
			SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
			   "Unsupported transparent material %s for obiect [%zu] %s\n",
			   m.material_name.c_str(), i, o.mesh_name.c_str());
		}
	}
	glm::vec3 light_pos = glm::vec3(0, 2, 0);
	float light_power = 5.f;
	// end load scene

	// init
	int width, height; float aspectRatio;
	SDL_GetWindowSize(window, &width, &height);
	aspectRatio = (float)width / (float)height;

	GLuint solid_prog; //, transparent_prog, composite_prog, screen_prog;
	if (!load_shader_program("data/shaders/solid.vert", "data/shaders/solid.frag", "solid", &solid_prog)) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Cannot load solid shader program\n");
		return;
	}
	GLint su_model, su_view, su_projection, su_normal_matrix_mv, su_light_pos_w, su_light_power, su_diffuse_map, su_normal_map, su_has_normal_map;
	{
		su_model = glGetUniformLocation(solid_prog, "model");
		su_view = glGetUniformLocation(solid_prog, "view");
		su_projection = glGetUniformLocation(solid_prog, "projection");
		su_normal_matrix_mv = glGetUniformLocation(solid_prog, "normal_matrix_mv");
		su_light_pos_w = glGetUniformLocation(solid_prog, "light_pos_w");
		su_light_power = glGetUniformLocation(solid_prog, "light_power");
		su_diffuse_map = glGetUniformLocation(solid_prog, "diffuse_map");
		su_normal_map = glGetUniformLocation(solid_prog, "normal_map");
		su_has_normal_map = glGetUniformLocation(solid_prog, "has_normal_map");
	}

	// end init
	// loop
	SDL_Event event;
	bool running = true, captured = true;

	player player{};
	double default_speed = player.speed;
	double sprint_speed = default_speed * 3.0;
	bool is_sprinting = false;

	glm::mat4 view;
	glm::mat4 proj = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
	SDL_SetRelativeMouseMode((SDL_bool)captured);
	Uint64 time_last, time_now; float delta_time;
	time_last = SDL_GetTicks64();
	char window_title[256];
	while (running) {
		{ /* delta_time */
			time_now = SDL_GetTicks64(); // in ms
			delta_time = (time_now - time_last) * 0.001; // ms to s
			time_last = time_now;
		}
		while (SDL_PollEvent(&event)) switch (event.type) {
			default: break;
			case SDL_QUIT: running = false; break;
			case SDL_WINDOWEVENT:
			switch (event.window.event) {
				default: break;
				case SDL_WINDOWEVENT_SIZE_CHANGED: {
					width = event.window.data1;
					height = event.window.data2;
					aspectRatio = (float)width / (float)height;
					proj = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
					glViewport(0, 0, width, height);
					SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Window resized to %d %d\n", width, height);
				} break;
			} break;
			case SDL_KEYDOWN: {
				if (!event.key.repeat) SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION,
					"Keydown scancode %s keycode %s\n",
					SDL_GetScancodeName(event.key.keysym.scancode),
					SDL_GetKeyName(event.key.keysym.sym));
				if (!event.key.repeat && event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
					SDL_WarpMouseInWindow(window, width / 2, height / 2);
					captured = !captured;
					SDL_SetRelativeMouseMode((SDL_bool)captured);
					if (!captured) { player.movement = glm::ivec3(0); }
				}
				if (captured) switch (event.key.keysym.scancode) {
					default: break;
					case SDL_SCANCODE_W: if (!event.key.repeat) player.movement.x += 1; break;
					case SDL_SCANCODE_S: if (!event.key.repeat) player.movement.x -= 1; break;
					case SDL_SCANCODE_A: if (!event.key.repeat) player.movement.y -= 1; break;
					case SDL_SCANCODE_D: if (!event.key.repeat) player.movement.y += 1; break;
					case SDL_SCANCODE_SPACE: if (!event.key.repeat) player.movement.z += 1; break;
					case SDL_SCANCODE_LSHIFT: if (!event.key.repeat) player.movement.z -= 1; break;
					case SDL_SCANCODE_LEFT: light_pos.x += 0.1; break;
					case SDL_SCANCODE_RIGHT: light_pos.x -= 0.1; break;
					case SDL_SCANCODE_UP: light_pos.z += 0.1; break;
					case SDL_SCANCODE_DOWN: light_pos.z -= 0.1; break;
					case SDL_SCANCODE_EQUALS: light_pos.y += 0.1; break;
					case SDL_SCANCODE_MINUS: light_pos.y -= 0.1; break;
					case SDL_SCANCODE_H: light_power -= 0.1; break;
					case SDL_SCANCODE_J: light_power += 0.1; break;
					case SDL_SCANCODE_TAB: if (!event.key.repeat) {
						is_sprinting = !is_sprinting;
						player.speed = is_sprinting ? sprint_speed : default_speed;
					} break;
				}
			} break;
			case SDL_KEYUP: {
				SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION,
					"Keyup scancode %s keycode %s\n",
					SDL_GetScancodeName(event.key.keysym.scancode),
					SDL_GetKeyName(event.key.keysym.sym));
				if (captured) switch (event.key.keysym.scancode) {
					default: break;
					case SDL_SCANCODE_W: player.movement.x -= 1; break;
					case SDL_SCANCODE_S: player.movement.x += 1; break;
					case SDL_SCANCODE_A: player.movement.y += 1; break;
					case SDL_SCANCODE_D: player.movement.y -= 1; break;
					case SDL_SCANCODE_SPACE: player.movement.z -= 1; break;
					case SDL_SCANCODE_LSHIFT: player.movement.z += 1; break;
				}
			} break;
			case SDL_MOUSEMOTION: if (captured) {
				float dx = event.motion.xrel;
				float dy = -event.motion.yrel; // change y direction
				player.rotate_camera(dx, dy);
			} break;
			case SDL_DROPFILE: {
				char *filename = event.drop.file;
				SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, "File '%s' dropped\n", filename);
				// здесь буду грузить объекты если что (2025-05-30 16:03 UTC+0300)
				SDL_free(filename);
			} break;
		}
		player.move(delta_time);
		bool inside_trigger;
		{
			glm::vec3 p = player.position;
			// пока так
			glm::vec3 origin = glm::vec3(1,1,1), end = glm::vec3(2,2,2);
			bool in_x = (p.x >= origin.x && p.x <= end.x);
			bool in_y = (p.y >= origin.y && p.y <= end.y);
			bool in_z = (p.z >= origin.z && p.z <= end.z);
			inside_trigger = in_x && in_y && in_z;
		}

		objects[0].transform.set_position(light_pos);

		// SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "position x y z: %.2f %.2f %.2f\nrotation yaw pitch: %.2f %.2f\n", player.position.x, player.position.y, player.position.z, player.rotation.x, player.rotation.y);
		snprintf(
			window_title, sizeof(window_title),
			"%sposition: %.2f %.2f %.2f | rotation: %.2f %.2f | light: %.2f %.2f %.2f %.2f",
			inside_trigger ? "[Target] " : "",
			player.position.x, player.position.y, player.position.z,
			player.rotation.x, player.rotation.y,
			light_pos.x, light_pos.y, light_pos.z, light_power
		);
		SDL_SetWindowTitle(window, window_title);

		view = glm::lookAt(player.position, player.position + player.front(), player.up());

		glEnable(GL_CULL_FACE);
		// glDisable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glClearColor(0.7, 0.7, 0.9, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// glClear(GL_DEPTH_BUFFER_BIT);
		
		for (size_t i = 0; i < objects_len; i++) {
			object &o = objects[i];
			if (!o.has_mesh()) continue;
			mesh &m = o.mesh();
			if (!m.has_material()) continue;
			material &mm = m.material();
			if (mm.is_transparent) continue;
			glUseProgram(solid_prog);
			glUniformMatrix4fv(su_view, 1, false, glm::value_ptr(view));
			glUniformMatrix4fv(su_projection, 1, false, glm::value_ptr(proj));
			glm::mat3 normal_matrix_mv = glm::inverseTranspose(glm::mat3(view * o.transform.model_matrix()));
			glUniformMatrix4fv(su_model, 1, false, glm::value_ptr(o.transform.model_matrix()));
			glUniformMatrix3fv(su_normal_matrix_mv, 1, false, glm::value_ptr(normal_matrix_mv));
			glUniform3fv(su_light_pos_w, 1, glm::value_ptr(light_pos));
			glUniform1f(su_light_power, light_power);
			glActiveTexture(GL_TEXTURE0); // TEXTURE0 = 0
			glBindTexture(GL_TEXTURE_2D, textures.at(mm.diffuse_texture.value()));
			glUniform1i(su_diffuse_map, 0); // TEXTURE0 = 0
			if (mm.normal_texture.has_value()) {
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, textures.at(mm.normal_texture.value()));
				glUniform1i(su_normal_map, 1);
				glUniform1i(su_has_normal_map, 1);
			}
			else {
				glUniform1i(su_has_normal_map, 0);
			}
			glBindVertexArray(m.vao);
			glDrawArrays(GL_TRIANGLES, 0, m.size());
		}

		// todo: ограничение кадов
		SDL_GL_SwapWindow(window);
		// SDL_Delay(Uint32 ms); delay after swap
	}
	// end loop
}
