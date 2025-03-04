#version 330 core

layout (triangles) in; // 从顶点着色器输入 图元
layout (line_strip, max_vertices = 2) out; // 设定输出图元类型和最大顶点数

in VS_OUT{
	vec3 normal;
}gmIn[];

uniform mat4 projection_;

out vec3 _normalDispalcement_; // 法线移动的向量
void GetLine(int index) {
	gl_Position = gl_in[index].gl_Position; // 可视法线起点, 观察空间 -> NDC
	EmitVertex();
	_normalDispalcement_ = gmIn[index].normal * 0.1f;
	gl_Position = projection_* (gl_in[index].gl_Position + vec4(_normalDispalcement_, 0.f)); // NDC
	EmitVertex();
}

void main() {
	GetLine(0);
	GetLine(1);
	GetLine(2);

	EndPrimitive();
}