#version 330 core

struct GBuffer {
	sampler2D texturePosition;
	sampler2D textureNormal;
	sampler2D textureAlbedoSpec;
};

struct Attenuation {
	float a; // 二次项
	float b; // 一次项
	float c; // 常数项
};

in vec2 vTexCoords;
out vec4 fColor;

uniform vec3 uLightPosition;
uniform vec3 uLightColor;
uniform Attenuation uAttenuation;

uniform GBuffer uGBuffer;
uniform sampler2D uTextureOcclusion;

void main() {
	vec3 position = texture(uGBuffer.texturePosition, vTexCoords).rgb;
	vec3 normal = texture(uGBuffer.textureNormal, vTexCoords).rgb;

	vec3 diffColorMaterial = vec3(texture(uGBuffer.textureAlbedoSpec, vTexCoords).r);
	float specIntensity = texture(uGBuffer.textureAlbedoSpec, vTexCoords).g;

	vec3 posToCamera = normalize(vec3(0.f) - position);
	vec3 posToLight = uLightPosition - position;
	
	float distancePos_Lit = length(posToLight);
	float attenuation = 1.f / (uAttenuation.c + uAttenuation.b * distancePos_Lit  + uAttenuation.a * distancePos_Lit * distancePos_Lit);

	posToLight = normalize(posToLight);
	
	float occlusion = texture(uTextureOcclusion, vTexCoords).r;
	vec3 colorAmbient = 1.f * uLightColor * (1.f - occlusion);

	float diff = max(dot(posToLight, normal), 0.f);
	vec3 colorDiff = diffColorMaterial * uLightColor * diff * attenuation;

	vec3 bisector = normalize(posToCamera + posToLight);
	float spec = pow(max(dot(bisector, normal), 0.f), 8);
	vec3 colorSpec = uLightColor * spec * attenuation; // 这里测试以下示例

	vec3 colorResult = colorAmbient + colorDiff + colorSpec;
	//vec3 colorResult = colorDiff + colorSpec;
	colorResult = pow(colorResult, vec3(1.f / 2.2));
	fColor = vec4(colorResult, 1.f);
	//fColor = vec4(vec3(specIntensity), 1.f);
}
