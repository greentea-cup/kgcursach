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

struct model {
	glm::vec3 position = glm::vec3(0, 0, 0);
	glm::vec3 scale = glm::vec3(1, 1, 1);
	glm::quat rotation = glm::quat(glm::vec3(0, 0, 0));
	glm::mat4 get_model_matrix() {
		glm::mat4 scaleM = glm::scale(glm::mat4(1.0f), scale);
		glm::mat4 rotateM = glm::mat4_cast(rotation);
		glm::mat4 translateM = glm::translate(glm::mat4(1.0f), position);
		return translateM * rotateM * scaleM;
	}
};

struct player {
	static float pitch_min; 
	static float pitch_max;
	static float mouse_sensitivity;

	glm::vec3 position = glm::vec3(0, 0, 0);
	// x = yaw (rad), y = pitch (rad)
	glm::vec2 rotation = glm::vec2(0, 0);
	glm::ivec3 movement = glm::ivec3(0, 0, 0);;
	float speed = 0.10f;

	glm::vec3 front() {
		glm::vec3 dir;
		dir.x = glm::cos(rotation.x) * glm::cos(rotation.y);
		dir.y = glm::sin(rotation.y);
		dir.z = glm::sin(rotation.x) * glm::cos(rotation.y);
		return glm::normalize(dir);
	}
	glm::vec3 up() {
		return glm::vec3(0, 1, 0);
	}
	void rotate_camera(float dx, float dy) {
		rotation.x = fmod(rotation.x + dx * mouse_sensitivity, 2 * glm::pi<float>());
		rotation.y = glm::clamp(rotation.y + dy * mouse_sensitivity, pitch_min, pitch_max);
	}
	
	void move(float delta_time) {
		if (movement.x == 0 && movement.y == 0 && movement.z == 0) return;
		glm::vec3 direction = glm::vec3(0);
		glm::vec3 f = front(), u = up();
		glm::vec3 r = glm::normalize(glm::cross(f, u));
		if (movement.x != 0) direction += (movement.x * 1.f) * f;
		if (movement.y != 0) direction += (movement.y * 1.f) * r;
		if (movement.z != 0) direction += (movement.z * 1.f) * u;
		position += glm::normalize(direction) * delta_time;
	}
};
float player::pitch_min = glm::radians(-89.5f);
float player::pitch_max = glm::radians(+89.5f);
float player::mouse_sensitivity = glm::pi<float>() * (1/180.f) * 0.1f; // degrees(1.0f) is 1 radian in degrees

void mainloop(SDL_Window *window) {
	std::optional<mesh> m1 = mesh::from_obj_file("data/objects/dice.obj");
	model m1_o1;
	// init
	int width, height; float aspectRatio;
	SDL_GetWindowSize(window, &width, &height);
	aspectRatio = (float)width / (float)height;

	size_t m1_size = 0;

	GLuint model1_vao, model1_vertices, model1_uvs, model1_normals;
	glGenVertexArrays(1, &model1_vao);
	glGenBuffers(1, &model1_vertices); glGenBuffers(1, &model1_uvs); glGenBuffers(1, &model1_normals);
	glBindVertexArray(model1_vao);
	if (m1.has_value()) {
		m1_size = m1->vertices.size() * 3;
		SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Successfully loaded mesh '%s' (len %zu)\n", m1->name.c_str(), m1_size);
		glBindBuffer(GL_ARRAY_BUFFER, model1_vertices);
		glBufferData(GL_ARRAY_BUFFER, m1->vertices.size() * sizeof(*m1->vertices.data()), m1->vertices.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);

		glBindBuffer(GL_ARRAY_BUFFER, model1_uvs);
		glBufferData(GL_ARRAY_BUFFER, m1->uvs.size() * sizeof(*m1->uvs.data()), m1->uvs.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, false, 0, 0);

		glBindBuffer(GL_ARRAY_BUFFER, model1_normals);
		glBufferData(GL_ARRAY_BUFFER, m1->normals.size() * sizeof(*m1->normals.data()), m1->normals.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, false, 0, 0);
	}
	glBindVertexArray(0);

	GLuint vao1, vbo1;
	glGenVertexArrays(1, &vao1);
	glGenBuffers(1, &vbo1);

	glBindVertexArray(vao1); // bind 'vao' as active vertex array object special target
	// glBindBuffer(GL_ARRAY_BUFFER) is not modifying vao state
	glBindBuffer(GL_ARRAY_BUFFER, vbo1); // bind some buffer vbo into GL_ARRAY_BUFFER
	float vertices1[] = {
		0, 0, 0,
		1, 1, 0, 
		0, 1, 0,
		1, 1, 0, 
		0, 1, 0,
		0, 0, 0,
	};
	// copy date from vertices into buffer at GL_ARRAY_BUFFER
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices1), vertices1, GL_STATIC_DRAW);
	// retreive data from GL_ARRAY_BUFFER and bind it at 'index' for current vao
	// index, size (1,2,3,4,GL_BGRA), type, normalized?, stride, offset
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0,  0);
	glEnableVertexAttribArray(0); // enable attribute 0 for vao
	glBindBuffer(GL_ARRAY_BUFFER, 0); // bind 0 which is null-obj in core profile
	glBindVertexArray(0);

	GLuint vao2;
	glGenVertexArrays(1, &vao2);
	GLuint vbo2; // GLuint vbo3;
	glGenBuffers(1, &vbo2); // glGenBuffers(1, &vbo3);
	glBindVertexArray(vao2);
	glBindBuffer(GL_ARRAY_BUFFER, vbo2);
	float vertices2[] = {
		 0,  0, 0,
		-1,  0, 0,
		-1, -1, 0,
		-1,  0, 0,
		 0,  0, 0,
		-1, -1, 0,
	};
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	GLuint main_prog;
	if (!load_shader_program("data/shaders/main.vert", "data/shaders/main.frag", "main", &main_prog)) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Cannot load main shader program\n");
		return;
	}
	GLint main_u_model, main_u_view, main_u_projection;
	main_u_model = glGetUniformLocation(main_prog, "model");
	main_u_view = glGetUniformLocation(main_prog, "view");
	main_u_projection = glGetUniformLocation(main_prog, "projection");

	// end init
	// loop
	SDL_Event event;
	bool running = true, captured = true;

	player player;

	glm::mat4 view; // = glm::mat4(1.0f);
	glm::mat4 proj = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
	// view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f)); // (-a, -b, -c)
	SDL_SetRelativeMouseMode((SDL_bool)captured);
	Uint64 time_last, time_now; float delta_time;
	time_last = SDL_GetTicks64();
	char window_title[256];
	model mm1, mm2;
	// model matrix is one per model (scene object)
	glm::mat4 model1, model2;
	model1 = mm1.get_model_matrix();
	model2 = mm2.get_model_matrix();
	while (running) {
		time_now = SDL_GetTicks64(); // in ms
		delta_time = (time_now - time_last) * 0.001; // ms to s
		time_last = time_now;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
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
					SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
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
					}
				} break;
				case SDL_KEYUP: {
					SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
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
					char * filename = event.drop.file;
					SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "File '%s' dropped\n", filename);
					// здесь буду грузить объекты если что (2025-05-30 16:03 UTC+0300)
					SDL_free(filename);
				} break;
			}
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
		glClearColor(0.1, 0.1, 0.1, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);
	
		glUseProgram(main_prog);
		glUniformMatrix4fv(main_u_view, 1, false, glm::value_ptr(view));
		glUniformMatrix4fv(main_u_projection, 1, false, glm::value_ptr(proj));

		// glBindVertexArray(vao1);
		// glUniformMatrix4fv(main_u_model, 1, false, glm::value_ptr(model1));
		// glDrawArrays(GL_TRIANGLES, 0, sizeof(vertices1) / sizeof(*vertices1) / 3);
		//
		// glBindVertexArray(vao2);
		// glUniformMatrix4fv(main_u_model, 1, false, glm::value_ptr(model2));
		// glDrawArrays(GL_TRIANGLES, 0, sizeof(vertices2) / sizeof(*vertices2) / 3);
		
		glBindVertexArray(model1_vao);
		glUniformMatrix4fv(main_u_model, 1, false, glm::value_ptr(m1_o1.get_model_matrix()));
		glDrawArrays(GL_TRIANGLES, 0, m1_size);

		// todo: ограничение кадов
		SDL_GL_SwapWindow(window);
		// SDL_Delay(Uint32 ms); delay after swap
	}
	// end loop
}
