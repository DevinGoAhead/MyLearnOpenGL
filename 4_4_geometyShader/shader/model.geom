#version 330 core

layout (points) in; // 从顶点着色器输入点图元
layout (triangle_strip, max_vertices = 5) out; // 设定输出图元类型和最大顶点数

in VS_OUT{
	vec3 color;
}gmIn[]; // 因为几何着色器是作用于输入的一组顶点的，从顶点着色器发来输入数据总是会以数组的形式表示出来，即便我们现在只有一个顶点

out vec3 _color_;

void main() {
	_color_ = gmIn[0].color;
	vec4 position = gl_in[0].gl_Position;
	gl_Position = position + vec4(-0.2, -0.2, 0.0, 0.0); // 1:左下
	EmitVertex();

	gl_Position = position + vec4( 0.2, -0.2, 0.0, 0.0); // 2:右下
	EmitVertex();

	gl_Position = position + vec4(-0.2, 0.2, 0.0, 0.0); // 3:左上
	EmitVertex();

	gl_Position = position + vec4( 0.2, 0.2, 0.0, 0.0); // 4:右上
	EmitVertex();

	gl_Position = position + vec4( 0.0, 0.4, 0.0, 0.0); // 5:顶部
	_color_ = vec3( 1.f, 1.f, 1.f);
	EmitVertex();

	EndPrimitive(); // 结束
}