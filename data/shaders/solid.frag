#version 330 core

in vec2 uv;
in mat3 TBN;
in vec3 frag_pos_mv;
in vec3 normal_mv;
in vec3 light_pos_mv;
in vec3 frag_pos_w;

out vec4 color;

uniform vec3 view_pos_w;
uniform vec3 light_pos_w;
// uniform vec3 lights_pos_mv[16];
uniform sampler2D diffuse_map;
uniform sampler2D normal_map;
uniform bool has_normal_map;
const vec3 light_color = vec3(.88, 0.9, 0.75);
uniform float light_power = 1.;
const ivec3 light_intensity = ivec3(1, 1, 1);

void main() {
	vec4 diffuse_color_a = texture(diffuse_map, uv);
	vec3 diffuse_color = diffuse_color_a.rgb;
	float alpha = diffuse_color_a.a;
	if (alpha < 0.1) discard;

	vec3 ambient_color = vec3(0.05) * diffuse_color;
	vec3 specular_color = vec3(0.3);

	vec3 normal;
	if (has_normal_map) {
		vec3 tx_normal = normalize(texture(normal_map, uv).rgb) * 2. - 1.;
		normal = normalize(TBN * tx_normal);
	}
	else {
		normal = normalize(TBN[2]);
	}
	// normal = normalize(TBN[2]);
	vec3 light_rel_mv = light_pos_mv - frag_pos_mv;
	float distance = length(light_rel_mv);
	vec3 light_dir_mv = light_rel_mv / distance;
	float distance2 = distance * distance;
	float q = 1. / distance2;
	vec3 light_mult = light_color * light_power * q;

	vec3 ambient = ambient_color;

	float diff = max(dot(normal, light_dir_mv), 0.);
	vec3 diffuse = diffuse_color * diff * light_mult;
	
	vec3 view_dir_mv = normalize(-frag_pos_mv);
	vec3 halfway_dir_mv = normalize(light_dir_mv + view_dir_mv);
	float spec = pow(max(dot(normal, halfway_dir_mv), 0.), 32.0);
	vec3 specular = specular_color * spec * light_mult;

	vec3 color3;
	color3 = ambient + diffuse + specular;
	color = vec4(color3, alpha);
}
