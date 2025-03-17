#version 330 core
layout (location = 0) out vec4 fragColor; // 常规颜色输出, COLOR_ATTACHMENT0
layout (location = 1) out vec4 brightColor; // HDR 颜色输出, COLOR_ATTACHMENT1

uniform vec3 uLightColor;

void main() {
	fragColor = vec4(uLightColor, 1.f);
	// 筛选出 0.2126 * R + 0.7152 * G + 0.0722 * B > 1.f 的光, 即根据人眼敏感度加权求和后亮度 > 1 的像素
	brightColor = (dot(uLightColor, vec3(0.2126, 0.7152, 0.0722)) > 1.f ? vec4(uLightColor, 1.f) : vec4(vec3(0.f), 1.f));
	//fragColor = brightColor;
}