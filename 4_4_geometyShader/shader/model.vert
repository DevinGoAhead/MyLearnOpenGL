#version 330 core

layout (location = 0) in vec3 vertex_v;
layout (location = 1) in vec3 normal_v;
layout (location = 2) in vec2 texCoords_v;

out vec2 _texCoords_;
out vec3 _normal_;
out vec3 _position_;

uniform mat4 model_;
uniform mat4 view_;
uniform mat4 projection_;

void main() {
	gl_Position = projection_ * view_ * model_ * vec4(vertex_v, 1.0f);// NDC
	_position_ = vec3(model_ *  vec4(vertex_v, 1.0f));// 世界空间
	_normal_ = mat3(transpose(inverse(model_))) * normal_v; //世界空间
	_texCoords_ = texCoords_v;
}