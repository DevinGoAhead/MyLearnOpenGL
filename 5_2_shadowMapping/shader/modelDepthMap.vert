#version 330 core
layout (location = 0) in vec3 aPos;

// 这些都是以 光源为基准的变换矩阵
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

void main() {
	gl_Position = uProjection * uView * uModel * vec4(aPos, 1.f);
}
