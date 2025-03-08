#version 330 core

in vec2 vTexCoord;
out vec4 fColor;

uniform sampler2D uScreenTexture;

void main() {
	float depth = texture(uScreenTexture, vTexCoord).r; // 仅 r 分量有效
	fColor = vec4(vec3(depth), 1.f);
}