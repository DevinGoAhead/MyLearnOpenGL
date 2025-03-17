#version 330 core

in vec2 vTexCoords;
out vec4 fColor;

uniform float uWeight[5];
uniform int uHorizontal;
uniform sampler2D uTextureBright;

vec3 GaussianBlur(vec2 dTexCoords) {
	vec3 resultColor = vec3(0.f);
	for(int i = 0; i < 5; ++i) {
		resultColor += texture(uTextureBright, vTexCoords - dTexCoords * i).rgb * uWeight[i];
		resultColor += texture(uTextureBright, vTexCoords + dTexCoords * i).rgb * uWeight[i];
	}
	return resultColor;
}
void main() {
	vec2 texSize = 1.f / textureSize(uTextureBright, 0); // 获取第 0 级 MipMap 纹理的单位尺寸

	if(uHorizontal == 1) fColor = vec4(GaussianBlur(vec2(texSize.x, 0.f)), 1.f);
	else fColor = vec4(GaussianBlur(vec2(0.f, texSize.y)), 1.f);
}