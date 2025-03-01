#version 330 core

out vec4 _fragColor_;
in vec3 _TexCoords_;

uniform samplerCube texerSkyBox_;

void main()
{
	_fragColor_ = texture(texerSkyBox_, _TexCoords_);
	//_fragColor_ = vec4(0.6f);
}