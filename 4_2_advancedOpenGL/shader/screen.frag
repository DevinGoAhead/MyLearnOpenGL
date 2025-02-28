#version 330 core

out vec4 _fragColor_;
in vec2 _verTexCoords_;

uniform sampler2D texturerScreen_;

void main()
{
	vec3 col = 1.f - texture(texturerScreen_, _verTexCoords_).rgb;
	//col = vec3(1.f - col.x, 1.f - col.y, 1.f - col.z);
	_fragColor_ = vec4(col, 1.f);
	//_fragColor_ = vec4(1.f);
}