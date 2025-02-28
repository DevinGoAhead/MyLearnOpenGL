#version 330 core

out vec4 _fragColor_;
in vec2 _verTexCoords_;

uniform sampler2D texturerScreen_;

void main()
{
	vec3 col = texture(texturerScreen_, _verTexCoords_).rgb;
	//col = vec3((col.x + col.y + col.z) / 3); // 灰度
	col = vec3(0.2 * col.r + 0.7 * col.g + 0.1 * col.b); // 灰度
	_fragColor_ = vec4(col, 1.f);
}