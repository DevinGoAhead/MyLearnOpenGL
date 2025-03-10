#version 330 core
layout (location = 0) in vec3 aPos;

// 这些都是以 光源为基准的变换矩阵
uniform mat4 uModel;

void main() {
	gl_Position = uModel * vec4(aPos, 1.f);
}
