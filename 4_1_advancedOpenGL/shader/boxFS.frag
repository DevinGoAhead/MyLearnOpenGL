#version 330 core

out vec4 _fragColor_;
in vec2 _verTexCoords_;

uniform sampler2D texturer0_;

void main()
{
	float near = 0.1f;
	float far = 100.f;
	float z = gl_FragCoord.z; // 屏幕空间

	//屏幕空间中的 z 的范围是[0, 1], 若变换到标准设备空间 NDC 中
	z = z * 2 -1; // 先变换到[0, 2], 再 - 1变换到[-1, 1]

	// 变换回线性空间, 推导详见 games101, typora 笔记
	z = (2 * near * far) / ((near + far) + z * (near - far));

	// 此时 z 是在[0.1,100] 范围，但 RGB 需要在[0,1] 的范围，这里简单的做一个线性缩小
	z = z / far;

	_fragColor_ = vec4(vec3(z), 1.f);
}