#version 330 core

layout (location = 0) in vec3 vertex_v;
layout (location = 1) in vec3 normal_v;

uniform mat4 model_;
uniform mat4 view_;
uniform mat4 projection_;

out vec3 _vertex_;
out vec3 _normal_;

void main()
{
	gl_Position = projection_ * view_ * model_ * vec4(vertex_v, 1.0f);// 转换为齐次坐标

	_vertex_ = vec3(model_ * vec4(vertex_v, 1.0f)); // 世界坐标系
	_normal_ = mat3(transpose(inverse(model_))) * normal_v; // 世界坐标, 法线变换 - 顶点变换逆转置
}