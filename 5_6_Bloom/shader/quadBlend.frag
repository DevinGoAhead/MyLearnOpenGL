#version 330 core

out vec4 fColor;
in vec2 vTexCoords;

uniform sampler2D uHDRTexture;
uniform sampler2D uBlurTexture;
uniform float uExposure;

void main() {
	vec3 colorHDR = texture(uHDRTexture, vTexCoords).rgb;
	vec3 colorBlur = texture(uBlurTexture, vTexCoords).rgb;
	vec3 color = colorHDR + colorBlur; // 黑色
	//vec3 color = colorHDR; // 可以得到合理的渲染结果
	color = vec3(1.0) - exp(-color * uExposure); // exp(x) = e^x
	color = pow(color, vec3(1.f / 2.2)); // gamma correction
	fColor = vec4(color, 1.f);
}