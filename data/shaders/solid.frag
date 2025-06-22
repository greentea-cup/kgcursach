#version 330 core

in vec2 uv;
in mat3 TBN;
in vec3 frag_pos_mv;
in vec3 normal_mv;
in vec3 light_pos_mv;

out vec4 color;

#define LIGHTS_COUNT 16
uniform vec3 lights_pos_mv[LIGHTS_COUNT];
uniform vec3 lights_color[LIGHTS_COUNT];
uniform float lights_power[LIGHTS_COUNT];
uniform vec3 diffuse_color;
uniform sampler2D diffuse_map;
uniform bool has_diffuse_map;
uniform sampler2D normal_map;
uniform bool has_normal_map;

vec3 calc_light(
	int i, vec3 frag_pos_mv, vec3 view_dir_mv, vec3 normal,
	vec3 material_ambient, vec3 material_diffuse, vec3 material_specular
) {
	vec3 light_rel_mv, light_dir_mv, light_mult, halfway_dir_mv;
	light_rel_mv = lights_pos_mv[i] - frag_pos_mv;
	float distance = length(light_rel_mv);
	light_dir_mv = light_rel_mv / distance;
	light_mult = lights_color[i] * lights_power[i] / (distance * distance);
	halfway_dir_mv = normalize(light_dir_mv + view_dir_mv);

	float diff = max(dot(normal, light_dir_mv), 0.);
	float spec = pow(max(dot(normal, halfway_dir_mv), 0.), 32.0);

	vec3 ambient = material_ambient;
	vec3 diffuse = material_diffuse * diff * light_mult;
	vec3 specular = material_specular * spec * light_mult;

	return ambient + diffuse + specular;
}

void main() {
	vec3 material_diffuse, material_ambient, material_specular, normal;

	if (has_diffuse_map) material_diffuse = texture(diffuse_map, uv).rgb;
	else material_diffuse = diffuse_color;
	material_ambient = vec3(0.05) * material_diffuse;
	material_specular = vec3(0.3);
	if (has_normal_map)
		normal = normalize(TBN*((texture(normal_map, uv).rgb)*2.-1.));
	else
		normal = TBN[2]; /* pre-normalized in solid.vert */
	vec3 view_dir_mv = normalize(-frag_pos_mv);

	vec3 res_color = vec3(0.);
	for (int i = 0; i < LIGHTS_COUNT; i++) {
		if (lights_power[i] == 0.) continue;
		res_color += calc_light(
			i, frag_pos_mv, view_dir_mv, normal,
			material_ambient, material_diffuse, material_specular);
	}

	// res_color = normal;
	color = vec4(res_color, 1.);
}
