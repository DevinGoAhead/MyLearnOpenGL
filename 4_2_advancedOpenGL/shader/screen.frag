#version 330 core

out vec4 _fragColor_;
in vec2 _verTexCoords_;

uniform sampler2D texturerScreen_;

void main() {
	float offset = 1 / 20;
	vec2 offsets[] = vec2[](
		vec2(-offset, offset),	vec2(0.f, offset),	vec2(offset, offset),
		vec2(-offset, 0.f),		vec2(0.f, 0.f),		vec2(offset, 0.f),
		vec2(-offset, -offset),	vec2(0.f, -offset),	vec2(offset, -offset)
	);

	//锐化核
	// float sharpenKernel[] = float[](...)
	float sharpenKernel[] = float[](
		-1.f,	-1.f,	-1.f,
		-1.f,	9.f,	-1.f,
		-1.f,	-1.f,	-1.f
	);
	
	// 这里会将纹理采样点计算和卷积同步进行
	vec2 texCoords[9]; // 纹理采样点
	vec3 color = vec3(0.f);
	for(int i = 0; i < 9; ++i) {
		texCoords[i] = _verTexCoords_.st + offsets[i]; // 纹理采样点
		color += vec3(texture(texturerScreen_, texCoords[i]) * sharpenKernel[i]);
	};

	_fragColor_ = vec4(color, 1.f);
	//_fragColor_ = texture(texturerScreen_, _verTexCoords_);
}