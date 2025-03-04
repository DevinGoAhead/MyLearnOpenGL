#version 330 core

in vec3 _normalDispalcement_;
out vec4 _fragColor_;

void main(){
	_fragColor_ = vec4(0.3f, 0.5f, 0.7f, 1.f);
	//_fragColor_ = vec4(_normalDispalcement_, 1.f);
}