#version 330 core

in vec2 vTexCoord;
out vec4 fColor;

uniform sampler2D uTexture;

void main() {
	fColor = texture(uTexture, vTexCoord);
}