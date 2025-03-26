#version 330 core

layout (location = 0) in vec3 aPos;

out vec3 vPosWd;

uniform mat4 uView;
uniform mat4 uProjection;

void main() {
	vPosWd = aPos; // 这里 cube 就是世界

	vec4 posClip = uProjection * mat4(mat3(uView)) * vec4(aPos, 1.f); // 去除 view 的平移部分么确保相机移动时, 背景不会移动
	gl_Position = posClip.xyww;// 确保透视投影后, cube的所有点的深度总是为1, 即无限远
}