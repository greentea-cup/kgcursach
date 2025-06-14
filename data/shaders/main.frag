#version 330 core

in vec2 uv;
in vec3 position_w;
in vec3 normal;
in vec3 eye;
in vec3 light;
out vec4 color;

// uniform float near;
// uniform float far;
uniform sampler2D sampler;
// uniform vec3 light_position_w;
// uniform float light_power;
// uniform ivec3 light_intensity;
// uniform bool draw_depth;
const vec3 light_position_w = vec3(5, 10, 3);
const vec3 light_color = vec3(.88, 0.9, 0.75);
const float light_power = 100;
const ivec3 light_intensity = ivec3(1, 1, 1);

void main() {
    vec4 tx = texture(sampler, uv);
	// vec4 tx = vec4(uv, 0, 1);
	// color = tx;
	// return;
    vec3 mDiffuse = tx.rgb;
    float alpha = tx.a;
    if (alpha < 0.1) discard;
    vec3 mAmbient = vec3(0.1, 0.1, 0.1) * mDiffuse;
    vec3 mSpecular = vec3(0.3, 0.3, 0.3);
	vec3 light_position_rel = light_position_w - position_w;
    float dist2 = dot(light_position_rel, light_position_rel);
    float cos0 = clamp(dot(normal, light), 0, 1);
    vec3 reflection = reflect(-light, normal);
    float cosA = clamp(dot(eye, reflection), 0, 1);
    vec3 q = light_color * light_power / dist2;
    vec3 colorA = light_intensity.x * mAmbient;
    vec3 colorD = light_intensity.y * mDiffuse * q * cos0;
    vec3 colorS = light_intensity.z * mSpecular * q * pow(cosA, 5);
    color = vec4(colorA + colorD + colorS, alpha);
}
