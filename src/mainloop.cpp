// vim: set fdm=indent :
#include <fstream>
#include <string>
#include <sstream>
#include <optional>
#include <vector>
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include "loader.hpp"
#include "registry.hpp"
#include "player.hpp"
#include "transform.hpp"
#include "shader.hpp"

void mainloop(SDL_Window *window) {
	register_mesh("dice");
	register_mesh("box");
	register_mesh("dungeon_wall");
	
	// load scene
	object objects[] = {
		{"Box"}, {"Wall"}, {"Wall"}, {"Wall"}, {"Wall"}, {"Wall"}, {"Dice"}, {"Dice"}, {"Dice"}, {"Dice"}, {"Dice"}, {"Dice"}
	};
	size_t objects_len = sizeof(objects) / sizeof(*objects);
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
		object &o = objects[i];
		if (!o.has_mesh()) continue;
		mesh &m = o.mesh();
		if (m.has_material() && m.material().is_transparent) {
			SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
			   "Unsupported transparent material %s for obiect [%zu] %s\n",
			   m.material_name.c_str(), i, o.mesh_name.c_str());
		}
		m.prepare_to_drawing();
	}
	point_lights lights;
	lights.add(glm::vec3( 0.0f,  2.0f,  0.0f), glm::vec3(0.88, 0.90, 0.75), 5.0f);
	lights.add(glm::vec3( 3.0f,  2.0f, -3.0f), glm::vec3(0.95, 0.10, 0.15), 5.0f);
	lights.add(glm::vec3(-3.0f,  2.0f, -3.0f), glm::vec3(0.05, 0.88, 0.12), 5.0f);
	lights.add(glm::vec3( 3.0f,  2.0f,  3.0f), glm::vec3(0.13, 0.10, 0.80), 5.0f);
	size_t selected_light = 0;
	// end load scene

	// init
	int width, height; float aspectRatio;
	SDL_GetWindowSize(window, &width, &height);
	aspectRatio = (float)width / (float)height;

	std::optional<solid_shader_program> solid0 = solid_shader_program::create("data/shaders/solid.vert", "data/shaders/solid.frag");
	if (!solid0.has_value()) return;
	solid_shader_program solid = solid0.value();
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
					case SDL_SCANCODE_1: if (!event.key.repeat) selected_light = 0; break;
					case SDL_SCANCODE_2: if (!event.key.repeat) selected_light = 1; break;
					case SDL_SCANCODE_3: if (!event.key.repeat) selected_light = 2; break;
					case SDL_SCANCODE_4: if (!event.key.repeat) selected_light = 3; break;
					case SDL_SCANCODE_LEFT: lights.positions_w[selected_light].x += 0.1; break;
					case SDL_SCANCODE_RIGHT: lights.positions_w[selected_light].x -= 0.1; break;
					case SDL_SCANCODE_UP: lights.positions_w[selected_light].z += 0.1; break;
					case SDL_SCANCODE_DOWN: lights.positions_w[selected_light].z -= 0.1; break;
					case SDL_SCANCODE_EQUALS: lights.positions_w[selected_light].y += 0.1; break;
					case SDL_SCANCODE_MINUS: lights.positions_w[selected_light].y -= 0.1; break;
					case SDL_SCANCODE_H: lights.powers[selected_light] -= 0.1; break;
					case SDL_SCANCODE_J: lights.powers[selected_light] += 0.1; break;
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

		snprintf(
			window_title, sizeof(window_title),
			"%sposition: %.2f %.2f %.2f | rotation: %.2f %.2f",
			inside_trigger ? "[Target] " : "",
			player.position.x, player.position.y, player.position.z,
			player.rotation.x, player.rotation.y
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
		
		lights.calc_positions_mv(view);
		for (size_t i = 0; i < objects_len; i++) {
			object &o = objects[i];
			if (!o.has_mesh()) continue;
			mesh &m = o.mesh();
			if (!m.has_material()) continue;
			material &mm = m.material();
			if (mm.is_transparent) continue;
			glm::mat4 model = o.transform.model_matrix();
			glm::mat3 normal_matrix_mv = glm::inverseTranspose(glm::mat3(view * model));
			glUseProgram(solid.program);
			solid.set_view(view);
			solid.set_projection(proj);
			solid.set_model(model);
			solid.set_normal_mv(normal_matrix_mv);
			solid.set_lights(lights);
			solid.set_material(mm);
			glBindVertexArray(m.vao);
			glDrawArrays(GL_TRIANGLES, 0, m.size());
		}

		// todo: ограничение кадов
		SDL_GL_SwapWindow(window);
		// SDL_Delay(Uint32 ms); delay after swap
	}
	// end loop
}
