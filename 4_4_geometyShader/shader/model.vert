#version 330 core

layout (location = 0) in vec2 vertex_v;
layout (location = 1) in vec3 color_v;

out VS_OUT{
	vec3 color;
}vsOut;

void main() {
	gl_Position = vec4(vertex_v, 0.f, 1.0f);// 转换为齐次坐标
	vsOut.color = color_v;
}