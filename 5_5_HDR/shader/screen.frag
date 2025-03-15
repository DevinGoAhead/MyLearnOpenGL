#version 330 core

out vec4 fColor;
in vec2 vTexCoord;

uniform sampler2D uHDRTexture;
uniform float uExposure;

void main() {
	vec3 color = texture(uHDRTexture, vTexCoord).rgb;
	//color = color / (color + vec3(1.f));
	color = vec3(1.0) - exp(-color * uExposure); // exp(x) = e^x
	color = pow(color, vec3(1.f / 2.2)); // gamma correction
	fColor = vec4(color, 1.f);
}