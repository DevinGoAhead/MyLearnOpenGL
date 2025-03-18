#version 330 core

out vec4 fColor;

uniform vec3 uLightColor;

void main() {
	fColor = vec4(uLightColor, 1.f);
}