#version 330 core
layout (location = 0) in vec3 aPos;

// 这些都是以 光源为基准的变换矩阵
uniform mat4 uModel;
uniform mat4 uProjViewAtLgt; // "光" 坐标系下的投影变换 * 视图变换

void main() {
	gl_Position = uProjViewAtLgt * uModel * vec4(aPos, 1.f);
}
