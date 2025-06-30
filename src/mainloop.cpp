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
	register_mesh("dungeon_2");
	register_mesh("campfire_plate");
	register_mesh("campfire_log");
	register_mesh("campfire_fire");
	
	// load scene
	object objects[] = {
		{"Box"}, {"Wall"}, {"Wall"}, {"Wall"}, {"Wall"}, {"Wall"}, {"Dice"}, {"Dice"}, {"Dice"}, {"Dice"}, {"Dice"}, {"Dice"},
		{"Dungeon"}, {"CampfirePlate"}, {"CampfireLog"}, {"CampfireFire"}
	};
	object &campfire_fire = objects[15];
	size_t objects_len = sizeof(objects) / sizeof(*objects);
	objects[0].transform.set_position(glm::vec3(20.0f, 1.0f, 0.0f));
	objects[0].transform.set_scale(glm::vec3(0.1f));
	objects[1].transform.set_position(glm::vec3(20.0f, 0.0f, 0.0f));
	objects[1].transform.set_scale(glm::vec3(10.0f));
	objects[2].transform.set_position(glm::vec3(20.0f, 1.0f, -5.0f));
	objects[2].transform.set_rotation(glm::quat(glm::vec3(glm::radians(90.0f), 0.0f, 0.0f)));
	objects[3].transform.set_position(glm::vec3(20.0f, 1.0f, 5.0f));
	objects[3].transform.set_rotation(glm::quat(glm::vec3(glm::radians(90.0f), glm::radians(180.0f), 0.0f)));
	objects[4].transform.set_position(glm::vec3(15.0f, 1.0f, 0.0f));
	objects[4].transform.set_rotation(glm::quat(glm::vec3(glm::radians(90.0f), glm::radians(90.0f), 0.0f)));
	objects[5].transform.set_position(glm::vec3(25.0f, 1.0f, 0.0f));
	objects[5].transform.set_rotation(glm::quat(glm::vec3(glm::radians(90.0f), glm::radians(-90.0f), 0.0f)));
	objects[6].transform.set_scale(glm::vec3(0.1f));
	objects[6].transform.set_position(glm::vec3(20.0f, 1.9f, 0.0f));
	objects[7].transform.set_scale(glm::vec3(0.1f));
	objects[7].transform.set_position(glm::vec3(20.0f, 1.7f, 0.0f));
	objects[8].transform.set_scale(glm::vec3(0.1f));
	objects[8].transform.set_position(glm::vec3(20.0f, 1.5f, 0.0f));
	objects[9].transform.set_position(glm::vec3(20.0f,  1.0f, 0.0f));
	objects[10].transform.set_position(glm::vec3(22.0f, 1.0f, 0.0f));
	objects[11].transform.set_position(glm::vec3(22.0f, 1.0f, 2.0f));

	objects[12].transform.set_position(glm::vec3(0.0f,  0.0f, 0.0f));
	objects[13].transform.set_position(glm::vec3(0.0f,  0.0f, 0.0f));
	objects[14].transform.set_position(glm::vec3(0.0f,  0.0f, 0.0f));
	campfire_fire.transform.set_position(glm::vec3(0.0f, -2.0f, 0.0f));

	SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Objects loaded");

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

	SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Objects prepared to drawing");

	point_lights lights;
	// костёр
	lights.add(glm::vec3(  0.0f,  1.0f,  0.0f), glm::vec3(1.00, 0.56, 0.05),  0.0f);
	// тестовая площадка
	lights.add(glm::vec3( 20.0f,  3.0f,  0.0f), glm::vec3(0.88, 0.90, 0.75),  5.0f);
	lights.add(glm::vec3( 23.0f,  3.0f, -3.0f), glm::vec3(0.95, 0.10, 0.15),  5.0f);
	lights.add(glm::vec3( 17.0f,  3.0f, -3.0f), glm::vec3(0.05, 0.88, 0.12),  5.0f);
	lights.add(glm::vec3( 23.0f,  3.0f,  3.0f), glm::vec3(0.13, 0.10, 0.80),  5.0f);
	// светлячки/огоньки
	lights.add(glm::vec3(  5.0f,  3.0f,  1.0f), glm::vec3(0.31, 0.93, 0.11),  0.1f);
	lights.add(glm::vec3(  2.0f,  2.0f,  4.0f), glm::vec3(0.31, 0.93, 0.11),  0.1f);
	lights.add(glm::vec3(  1.0f,  3.0f,  1.0f), glm::vec3(0.31, 0.93, 0.11),  0.1f);
	lights.add(glm::vec3( -1.0f,  3.0f, -5.0f), glm::vec3(0.31, 0.93, 0.11),  0.1f);
	size_t selected_light = 1;
	float const campfire_light_max_power = 10.0f;
	float &campfire_light_current_power = lights.powers[0];
	bool campfire_lit = false;
	// end load scene
	
	SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Lights loaded");

	bool freecam = false;

	// init
	int width, height; float aspectRatio;
	SDL_GetWindowSize(window, &width, &height);
	aspectRatio = (float)width / (float)height;

	std::optional<solid_shader_program> solid0 = solid_shader_program::create("data/shaders/solid.vert", "data/shaders/solid.frag");
	if (!solid0.has_value()) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Shader load error, aborting\n");
		return;
	}
	solid_shader_program solid = solid0.value();
	// end init
	// loop
	SDL_Event event;
	bool running = true, captured = true;

	SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Shaders loaded");

	player player{};
	float default_speed = player.speed;
	float sprint_speed = default_speed * 4.0f;
	bool is_sprinting = false;

	glm::mat4 view;
	glm::mat4 proj = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
	SDL_SetRelativeMouseMode((SDL_bool)captured);
	Uint64 time_last, time_now; float delta_time;
	time_last = SDL_GetTicks64();
	char window_title[256];
	SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Started running");
	while (running) {
		{ /* delta_time */
			time_now = SDL_GetTicks64(); // in ms
			delta_time = (time_now - time_last) * 0.001f; // ms to s
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
					case SDL_SCANCODE_SPACE: if (!event.key.repeat && freecam) player.movement.z += 1; break;
					case SDL_SCANCODE_LSHIFT: if (!event.key.repeat && freecam) player.movement.z -= 1; break;
					case SDL_SCANCODE_1: if (!event.key.repeat) selected_light = 1; break;
					case SDL_SCANCODE_2: if (!event.key.repeat) selected_light = 2; break;
					case SDL_SCANCODE_3: if (!event.key.repeat) selected_light = 3; break;
					case SDL_SCANCODE_4: if (!event.key.repeat) selected_light = 4; break;
					case SDL_SCANCODE_LEFT: lights.positions_w[selected_light].x += 0.1f; break;
					case SDL_SCANCODE_RIGHT: lights.positions_w[selected_light].x -= 0.1f; break;
					case SDL_SCANCODE_UP: lights.positions_w[selected_light].z += 0.1f; break;
					case SDL_SCANCODE_DOWN: lights.positions_w[selected_light].z -= 0.1f; break;
					case SDL_SCANCODE_EQUALS: lights.positions_w[selected_light].y += 0.1f; break;
					case SDL_SCANCODE_MINUS: lights.positions_w[selected_light].y -= 0.1f; break;
					case SDL_SCANCODE_E: if (!event.key.repeat) {
						campfire_lit = !campfire_lit;
						if (campfire_lit) {
							campfire_light_current_power = campfire_light_max_power;
							campfire_fire.transform.set_position(glm::vec3(0, 0, 0));
						}
						else {
							campfire_light_current_power = 0.0f;
							campfire_fire.transform.set_position(glm::vec3(0, -2, 0));
						}
					} break;
					case SDL_SCANCODE_P: if (!event.key.repeat) {
						freecam = !freecam;
						if (!freecam) {player.position.y = 2;}
						player.movement.y = 0;
					} break;
					case SDL_SCANCODE_H: lights.powers[selected_light] -= 0.1f; break;
					case SDL_SCANCODE_J: lights.powers[selected_light] += 0.1f; break;
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
					case SDL_SCANCODE_SPACE: if (freecam) player.movement.z -= 1; break;
					case SDL_SCANCODE_LSHIFT: if (freecam) player.movement.z += 1; break;
				}
			} break;
			case SDL_MOUSEMOTION: if (captured) {
				float dx = (float)event.motion.xrel;
				float dy = (float)-event.motion.yrel; // change y direction
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
		glClearColor(0.7f, 0.7f, 0.9f, 1.0f);
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
			glDrawArrays(GL_TRIANGLES, 0, (GLsizei)m.size());
		}

		// todo: ограничение кадов
		SDL_GL_SwapWindow(window);
		// SDL_Delay(Uint32 ms); delay after swap
		SDL_Delay(10);
	}
	// end loop
}
