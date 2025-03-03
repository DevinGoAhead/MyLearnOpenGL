#version 330 core

layout (location = 0) in vec2 vertex_v;

void main() {
	gl_Position = vec4(vertex_v, 0.f, 1.0f);// 转换为齐次坐标
	
}