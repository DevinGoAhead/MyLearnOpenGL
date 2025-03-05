#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aOffset;

out vec3 vColor;

void main() {
	gl_Position = vec4(aPos * gl_InstanceID / 100  + aOffset, 0.f,1.f);
	vColor = aColor;
}
