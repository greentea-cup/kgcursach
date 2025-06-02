#version 420 core
// shader inputs
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv_;
layout (location = 2) in vec3 normal_;

// model * view * projection matrix
uniform mat4 mvp;

out vec2 uv;
out vec3 normal;

void main() {
	gl_Position = mvp * vec4(position, 1.0f);
    uv = uv_;
    normal = normal_;
}
