#version 330 core

struct GBuffer {
	sampler2D texturePosition;
	sampler2D textureNormal;
	sampler2D textureAlbedoSpec;
};

in vec2 vTexCoords;
out vec4 fColor;

const int lightNr = 64;
uniform vec3 uLightPositions[lightNr];
uniform vec3 uLightColors[lightNr];
uniform vec3 uCameraPos;
uniform GBuffer uGBuffer;

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
		float attenuation = 1.f / (1.f + 0.7 * distancePos_Lit  + 1.8 * distancePos_Lit * distancePos_Lit);

		posToLight = normalize(posToLight);
		
		float diff = max(dot(posToLight, normal), 0.f);
		vec3 resultDiff = diffColorMaterial * uLightColors[i] * diff * attenuation;

		vec3 bisector = normalize(posToCamera + posToLight);
		float spec = pow(max(dot(bisector, normal), 0.f), 256);
		vec3 resultSpec = uLightColors[i] * spec * specIntensity * attenuation;

		colorResult += (resultDiff + resultSpec);
		//colorResult += (resultDiff);
	}
	colorResult = pow(colorResult, vec3(1.f / 2.2));
	fColor = vec4(colorResult, 1.f); // 符合预期
	//fColor = vec4(vec3(specIntensity), 1.f); // 符合预期
}
