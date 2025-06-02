#version 420 core

in vec2 uv;
in vec3 normal;

layout (location = 0) out vec4 accum;
layout (location = 1) out float reveal;

uniform vec3 ambient_color;
uniform vec3 diffuse_color;
uniform vec3 specular_color;
uniform sampler2D diffuse_texture;
uniform float dissolve;
uniform ivec3 opts;
// (texture == & 0b10, color == & 0b1)
// x = ambient
// y = diffuse
// z = specular

void main() {
    /* vec3 diffuse = (opts.y == 0) ? vec3(0) : (
        (((opts.y & 1) == 1) ? diffuse_color : vec3(1))
        * (((opts.y & 2) == 2) ? texture(diffuse_texture, uv).rgb : vec3(1))
    );*/
    vec3 diffuse_tx = opts.y == 1 ? 
        texture(diffuse_texture, uv).rgb : vec3(1);
    // vec3 diffuse_tx = texture(diffuse_texture, uv).rgb;
    // vec3 diffuse = diffuse_tx;
    vec3 diffuse = diffuse_color * diffuse_tx;
    // vec3 diffuse = vec3(dissolve);
    // vec3 diffuse = vec3(1);
	// weight function
	float weight = clamp(pow(min(1.0, dissolve * 10.0) + 0.01, 3.0) * 1e8 * pow(1.0 - gl_FragCoord.z * 0.9, 3.0), 1e-2, 3e3);
	// store pixel color accumulation
	accum = vec4(diffuse * dissolve, dissolve) * weight;	
	// store pixel revealage threshold
	reveal = dissolve;
}
