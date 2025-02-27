#version 330 core

out vec4 _fragColor_;
in vec2 _verTexCoords_;

uniform sampler2D texturer0_;

void main()
{
	vec4 texColor = texture(texturer0_, _verTexCoords_);
	if(texColor.a < 0.01f) {discard;} // .a 值alpha 分量, 如果透明则丢弃该frag
	_fragColor_ = texColor;
}