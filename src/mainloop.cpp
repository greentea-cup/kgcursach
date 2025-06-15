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

	// load scene
	object objects[] = {
		{"Dice"}, {"Box"}, {"Dice"}, {"Dice"}
	};
	objects[1].transform.position = glm::vec3(2.3, 0.3, 0.1);
	objects[1].transform.rotation = glm::quat(
		glm::vec3(glm::radians(10.0f), 0, glm::radians(10.0f)));
	objects[2].transform.position = glm::vec3(2, 0, 3);
	objects[3].transform.position = glm::vec3(-2.5, 0, -1);
	size_t objects_len = sizeof(objects) / sizeof(*objects);
	for (size_t i = 0; i < objects_len; i++) {
		objects[i].mesh().prepare_to_drawing();
	}
	// end load scene

	// init
	int width, height; float aspectRatio;
	SDL_GetWindowSize(window, &width, &height);
	aspectRatio = (float)width / (float)height;

	GLuint main_prog;
	if (!load_shader_program("data/shaders/main.vert", "data/shaders/main.frag", "main", &main_prog)) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Cannot load main shader program\n");
		return;
	}
	GLint main_u_model, main_u_view, main_u_projection, main_u_sampler;
	main_u_model = glGetUniformLocation(main_prog, "model");
	main_u_view = glGetUniformLocation(main_prog, "view");
	main_u_projection = glGetUniformLocation(main_prog, "projection");
	main_u_sampler = glGetUniformLocation(main_prog, "sampler");

	// end init
	// loop
	SDL_Event event;
	bool running = true, captured = true;

	player player{};
	double default_speed = player.speed;
	double sprint_speed = default_speed * 3.0;

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
			case SDL_KEYDOWN: if (!event.key.repeat) {
				SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION,
					"Keydown scancode %s keycode %s\n",
					SDL_GetScancodeName(event.key.keysym.scancode),
					SDL_GetKeyName(event.key.keysym.sym));
				if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
					SDL_WarpMouseInWindow(window, width / 2, height / 2);
					captured = !captured;
					SDL_SetRelativeMouseMode((SDL_bool)captured);
					if (!captured) { player.movement = glm::ivec3(0); }
				}
				if (captured) switch (event.key.keysym.scancode) {
					default: break;
					case SDL_SCANCODE_W: player.movement.x += 1; break;
					case SDL_SCANCODE_S: player.movement.x -= 1; break;
					case SDL_SCANCODE_A: player.movement.y -= 1; break;
					case SDL_SCANCODE_D: player.movement.y += 1; break;
					case SDL_SCANCODE_SPACE: player.movement.z += 1; break;
					case SDL_SCANCODE_LSHIFT: player.movement.z -= 1; break;
					case SDL_SCANCODE_TAB: player.speed = sprint_speed; break;
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
					case SDL_SCANCODE_TAB: player.speed = default_speed; break;
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

		// SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "position x y z: %.2f %.2f %.2f\nrotation yaw pitch: %.2f %.2f\n", player.position.x, player.position.y, player.position.z, player.rotation.x, player.rotation.y);
		snprintf(
			window_title, sizeof(window_title),
			"position: %.2f %.2f %.2f | rotation: %.2f %.2f",
			player.position.x, player.position.y, player.position.z,
			player.rotation.x, player.rotation.y
		);
		SDL_SetWindowTitle(window, window_title);

		view = glm::lookAt(player.position, player.position + player.front(), player.up());

		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glClearColor(0.1, 0.1, 0.1, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		for (size_t i = 0; i < objects_len; i++) {
			object &o = objects[i];
			glUseProgram(main_prog);
			glUniformMatrix4fv(main_u_view, 1, false, glm::value_ptr(view));
			glUniformMatrix4fv(main_u_projection, 1, false, glm::value_ptr(proj));
			glUniformMatrix4fv(main_u_model, 1, false, glm::value_ptr(o.transform.get_model_matrix()));
			glActiveTexture(GL_TEXTURE0); // TEXTURE0 = 0
			glBindTexture(GL_TEXTURE_2D, textures.at(o.mesh().material().diffuse_texture.value()));
			glUniform1i(main_u_sampler, 0); // TEXTURE0 = 0
			glBindVertexArray(o.mesh().vao);
			glDrawArrays(GL_TRIANGLES, 0, o.mesh().size());
		}

		// todo: ограничение кадов
		SDL_GL_SwapWindow(window);
		// SDL_Delay(Uint32 ms); delay after swap
	}
	// end loop
}
