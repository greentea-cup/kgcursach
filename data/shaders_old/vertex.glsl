#version 330 core

layout(location = 0) in vec3 vertexPositon_m;
layout(location = 1) in vec2 vertexUV;
layout(location = 2) in vec3 vertexNormal_m;

out vec2 UV;
out vec3 position_w;
out vec3 normal_c;
out vec3 eyeDirection_c;
out vec3 lightDirection_c;

uniform mat4 MVP;
uniform mat4 V;
uniform mat4 M;
uniform vec3 lightPosition_w;

void main() {
    gl_Position = MVP * vec4(vertexPositon_m, 1);
    UV = vertexUV;
	position_w = (M * vec4(vertexPositon_m, 1)).xyz;
    vec3 vertexPositon_c = (V * M * vec4(vertexPositon_m, 1)).xyz;
    eyeDirection_c = vec3(0, 0, 0) - vertexPositon_c;
    vec3 lightPosition_c = (V * vec4(lightPosition_w, 1)).xyz;
    lightDirection_c = lightPosition_c + eyeDirection_c;
    normal_c = (V * M * vec4(vertexNormal_m, 0)).xyz;
}
