#version 330 core
layout (location = 0) in vec3 position_m;
layout (location = 1) in vec2 uv_;
layout (location = 2) in vec3 normal_m;

out vec2 uv;
out vec3 position_w;
out vec3 normal;
out vec3 eye;
out vec3 light;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
// uniform vec3 light_position_w;

const vec3 light_position_w = vec3(5, 10, 3);

void main() {
	gl_Position = projection * view * model * vec4(position_m, 1);
	uv = uv_;
	position_w = (model * vec4(position_m, 1)).xyz;
    vec3 position_c = (view * model * vec4(position_m, 1)).xyz;
    vec3 eye_direction_c = vec3(0, 0, 0) - position_c;
    vec3 light_position_c = (view * vec4(light_position_w, 1)).xyz;
	eye = normalize(eye_direction_c);
    light = normalize(light_position_c + eye_direction_c);
	normal = normalize((view * model * vec4(normal_m, 0)).xyz);
}
