#version 330 core

layout (location = 0) in vec3 vertex_v;
layout (location = 1) in vec3 normal_v;

out VS_OUT{
	vec3 normal;
}vsOut; 

uniform mat4 model_;
uniform mat4 view_;

void main() {
	gl_Position = view_ * model_ * vec4(vertex_v, 1.0f);// 观察空间的点坐标
	vsOut.normal = normalize(mat3(transpose(inverse(view_ * model_))) * normal_v); // 观察空间的法线
}