#version 330 core
layout (location = 0) out vec4 fragColor; // 常规颜色输出, COLOR_ATTACHMENT0
layout (location = 1) out vec4 brightColor; // HDR 颜色输出, COLOR_ATTACHMENT1

struct Material {
	sampler2D textureDiffuse0;
};

in VOUT {
	vec3 pos;
	vec3 normal;
	vec2 texCoord;
}fIn;

uniform Material uMaterial;
uniform vec3 uLightPositions[4];
uniform vec3 ulightColors[4];

void main() {
	vec3 normal = normalize(fIn.normal);
	vec3 colorMaterial = texture(uMaterial.textureDiffuse0, fIn.texCoord).rgb; // 漫反射纹理
	vec3 colorResult = vec3(0.f);
	for(int i = 0; i < 4; ++i) {
		vec3 pointToLight = uLightPositions[i] - fIn.pos;
		float distancePtLit = length(pointToLight);
		float attenuation = 1.f / (distancePtLit * distancePtLit);

		pointToLight = normalize(pointToLight);
		
		float diff = max(dot(pointToLight, normal), 0.f);
		vec3 lightDiffuse = ulightColors[i] * diff * attenuation;

		colorResult += lightDiffuse;
	}
	colorResult *= colorMaterial;
	fragColor = vec4(colorResult, 1.f);
	//fragColor = vec4(vec3(0.f), 1.f);
	// 筛选出 0.2126 * R + 0.7152 * G + 0.0722 * B > 1.f 的光, 即根据人眼敏感度加权求和后亮度 > 1 的像素
	brightColor = (dot(colorResult, vec3(0.2126, 0.7152, 0.0722)) > 1.f ? vec4(colorResult, 1.f) : vec4(vec3(0.f), 1.f));
	//fragColor = brightColor;
}