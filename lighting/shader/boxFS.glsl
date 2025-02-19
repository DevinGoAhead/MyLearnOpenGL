#version 330 core

uniform vec3 boxColor_;
uniform vec3 lightColor_;
out vec4 fragColor_f;

void main()
{
	fragColor_f = vec4(boxColor_ * lightColor_, 1.f);
	//fragColor_f = vec4(0.6f,0.8f,0.8f,1.0f);
}