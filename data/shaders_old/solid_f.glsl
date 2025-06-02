#version 420 core

in vec2 uv;
in vec3 normal;

layout (location = 0) out vec4 color;

uniform float near;
uniform float far;
uniform vec3 ambient_color;
uniform vec3 diffuse_color;
uniform vec3 specular_color;
uniform sampler2D diffuse_texture;
uniform ivec3 opts;

float linear_depth(float depth) {
    float ndc = depth * 2.0 - 1.0;
    float res = (2.0 * near * far) / (far + near - ndc * (far - near));
    return res;
}

void main() {
    vec3 diffuse;
    if (opts.x == 1) { // depth
        diffuse = vec3(linear_depth(gl_FragCoord.z) / far);
    } else { // x == 0 (normal) or anything other
        diffuse = diffuse_color * texture(diffuse_texture, uv).rgb;
    }
    color = vec4(diffuse, 1.0);
}
