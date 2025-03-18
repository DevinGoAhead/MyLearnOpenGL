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

const int lightNr = 512;
uniform vec3 uLightPositions[lightNr];
uniform vec3 uLightColors[lightNr];
uniform float uLightRadius[lightNr];
uniform vec3 uCameraPos;
uniform GBuffer uGBuffer;
uniform Attenuation uAttenuation;


void main() {
	vec3 position = texture(uGBuffer.texturePosition, vTexCoords).rgb;
	vec3 normal = texture(uGBuffer.textureNormal, vTexCoords).rgb;
	vec3 diffColorMaterial = texture(uGBuffer.textureAlbedoSpec, vTexCoords).rgb; 
	float specIntensity = texture(uGBuffer.textureAlbedoSpec, vTexCoords).a; 

	vec3 posToCamera = normalize(uCameraPos - position);

	vec3 colorResult = vec3(0.f);
	for(int i = 0; i < lightNr ; ++i) {
		vec3 posToLight = uLightPositions[i] - position;
		float distancePos_Lit = length(posToLight);

		if(uLightRadius[i] < distancePos_Lit) {
			float attenuation = 1.f / (uAttenuation.c + uAttenuation.b * distancePos_Lit  + uAttenuation.a * distancePos_Lit * distancePos_Lit);

			posToLight = normalize(posToLight);
			
			float diff = max(dot(posToLight, normal), 0.f);
			vec3 resultDiff = diffColorMaterial * uLightColors[i] * diff * attenuation;

			vec3 bisector = normalize(posToCamera + posToLight);
			float spec = pow(max(dot(bisector, normal), 0.f), 256);
			vec3 resultSpec = uLightColors[i] * spec * specIntensity * attenuation;

			colorResult += (resultDiff + resultSpec);
			//colorResult += (resultDiff);
		}
	}
	colorResult = pow(colorResult, vec3(1.f / 2.2));
	fColor = vec4(colorResult, 1.f); // 符合预期
	//fColor = vec4(vec3(specIntensity), 1.f); // 符合预期
}
