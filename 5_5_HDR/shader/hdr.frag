#version 330 core

struct Material {
	sampler2D textureDiffuse0;
};

in VOUT {
	vec3 pos;
	vec3 normal;
	vec2 texCoord;
}fIn;

out vec4 fColor;

uniform Material uMaterial;
uniform vec3 uLightPoses[16];
uniform vec3 ulightColors[16];
uniform vec3 uCameraPos;

void main() {
	// 本节课提供的数据, 如果设置了 环境光和高光, 效果会很差
	vec3 normal = normalize(fIn.normal);
	if(!gl_FrontFacing) normal = -normal;
	vec3 colorMaterial = texture(uMaterial.textureDiffuse0, fIn.texCoord).rgb; // 漫反射纹理
	vec3 colorResult = vec3(0.f);
	for(int i = 0; i < 4; ++i) {
		vec3 point2Light = uLightPoses[i] - fIn.pos;
		float distancePtLit = length(point2Light);
		//float attenuation = 1.f / (1 + 0.04 *distancePtLit +  0.007 * distancePtLit * distancePtLit);
		float attenuation = 1.f / (distancePtLit * distancePtLit);
		point2Light = normalize(point2Light);
		
		float diff = max(dot(point2Light, normal), 0.f);
		vec3 lightDiffuse = ulightColors[i] * diff * attenuation;

		colorResult += lightDiffuse;
	}
	colorResult *= colorMaterial;
	fColor = vec4(colorResult, 1.f);
}