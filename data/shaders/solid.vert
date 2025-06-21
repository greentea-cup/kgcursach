#version 330 core
layout (location = 0) in vec3 frag_pos0;
layout (location = 1) in vec2 uv0;
layout (location = 2) in vec3 normal0;
layout (location = 3) in vec3 tangent0;

out vec2 uv;
out mat3 TBN;
out vec3 frag_pos_mv;
out vec3 normal_mv;
out vec3 light_pos_mv;
out vec3 frag_pos_w;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normal_matrix_mv;
uniform vec3 light_pos_w;
// uniform vec3 lights_pos_mv[16];

void main() {
	mat4 MV = view * model;
	gl_Position = projection * MV * vec4(frag_pos0, 1.0);

	uv = uv0;
	normal_mv = normalize(normal_matrix_mv * normal0);
	frag_pos_mv = (MV * vec4(frag_pos0, 1.0)).xyz;
	frag_pos_w = (model * vec4(frag_pos0, 1.0)).xyz;
	light_pos_mv = (view * vec4(light_pos_w, 1.0)).xyz;
	vec3 T = normalize(normal_matrix_mv * tangent0);
	vec3 N = normal_mv;
	T = normalize(T - dot(T, N) * N);
	vec3 B = normalize(cross(T, N));
	TBN = mat3(T, B, N);
}
