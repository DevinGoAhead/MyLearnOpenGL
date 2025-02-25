#version 330 core

uniform vec3 lightColor_;
out vec4 fragColor_f;

void main()
{
	fragColor_f = vec4(lightColor_, 1.f); // (1.f, 1.f, 1.f, 1.f)
}