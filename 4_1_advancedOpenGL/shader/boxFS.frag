#version 330 core

out vec4 _fragColor_;
in vec2 _verTexCoords_;

uniform sampler2D texturer0_;

void main()
{
	//_fragColor_ = texture(texturer0_, _verTexCoords_);
	_fragColor_ = vec4(vec3(gl_FragCoord.z), 1.f);
}