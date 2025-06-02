#version 330 core

in vec2 UV;
in vec3 position_w;
in vec3 normal_c;
in vec3 eyeDirection_c;
in vec3 lightDirection_c;

out vec4 color;

uniform float near;
uniform float far;
uniform sampler2D sampler;
uniform vec3 lightPosition_w;
uniform float lightPower;
uniform ivec3 lightIntensity;
uniform bool drawDepth;

float linear_depth(float depth) {
    float z = depth * 2 - 1;
    return (2 * near * far) / (far + near - z * (far - near));
}

void main() {
    if (drawDepth) {
        color = vec4(vec3(linear_depth(gl_FragCoord.z) / far), 1);
        return;
    }
    vec3 lightColor = vec3(1, 1, 1);
    vec4 tx = texture(sampler, UV);
    vec3 mDiffuse = tx.rgb;
    float alpha = tx.a;
    if (alpha < 0.1) discard;
    vec3 mAmbient = vec3(0.1, 0.1, 0.1) * mDiffuse;
    vec3 mSpecular = vec3(0.3, 0.3, 0.3);
    float dist = length(lightPosition_w - position_w);
    vec3 norm = normalize(normal_c);
    vec3 light = normalize(lightDirection_c);
    float cos0 = clamp(dot(norm, light), 0, 1);
    vec3 eye = normalize(eyeDirection_c);
    vec3 reflection = reflect(-light, norm);
    float cosA = clamp(dot(eye, reflection), 0, 1);
    float sqr_dist = dist * dist;
    vec3 colorA = lightIntensity.x * mAmbient;
    vec3 colorD = lightIntensity.y * (
            mDiffuse * lightColor * lightPower * cos0 / sqr_dist);
    vec3 colorS = lightIntensity.z * (
            mSpecular * lightColor * lightPower * pow(cosA, 5) / sqr_dist);
    color = vec4(colorA + colorD + colorS, alpha);
}
