#version 330 core

layout (location = 0) in vec3 aPos;

out vec3 vPosWd;

uniform mat4 uView;
uniform mat4 uProjection;

void main() {
	vPosWd = aPos; // 这里就是让cube 在场景中心, 测试而已
	gl_Position = uProjection *  uView * vec4(aPos, 1.f);
}