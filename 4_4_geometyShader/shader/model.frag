#version 330 core

out vec4 _fragColor_;
in vec3 _color_;

void main(){
	_fragColor_ = vec4(_color_, 1.f);
}