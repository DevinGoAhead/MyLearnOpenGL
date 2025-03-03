#version 330 core

layout (points) in; // 从顶点着色器输入点图元
layout (points, max_vertices = 1) out;

void main() {
	gl_Position = gl_in[0].gl_Position;
	gl_PointSize = 5.f;
	// 直接 emit, 不做任何处理
	EmitVertex();
	// 其它顶点
	EndPrimitive(); // 结束
}