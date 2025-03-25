#version 330 core

in VOUT {
	vec3 pos;
	vec3 normal;
	vec2 texCoords;
} fIn;

out vec4 fColor;

uniform float uRoughness; // 粗糙度
uniform vec3 uF0; // 基础反射率
uniform vec4 uMaterialColor; // 材质颜色
uniform float uMetalness; // 金属度
uniform vec3 uAlbedo; // 反照率
uniform float uAO; // 环境光遮蔽系数

uniform vec3 uCameraPosition;
uniform vec3 uLightPositions[4];
uniform vec3 uLightColors[4];

float pi = 3.14159265359;

float NDF_GGXTR(float NdotH) {
	float numer = uRoughness * uRoughness; // 分子

	// 分母
	float denom_ = (NdotH * NdotH) * (numer * numer - 1.f) + 1.f;
	float denom = pi * denom_ * denom_;
	return numer / denom + 0.0001;
}


float G_SchlickGGX(float Ndot, float k) {
	float numer = Ndot; // 分子

	// 分母
	float denom = numer * (1.f - k) + k;
	return numer / denom;
}

float G_Smith(float NdotV, float NdotL) {
	float k = (uRoughness + 1.f) * (uRoughness + 1.f) * 0.125;
	return  G_SchlickGGX(NdotV, k) * G_SchlickGGX(NdotL, k);
}

vec3 Fresnel_Schlick(float NdotV) {
	vec3 F0 = mix(uF0, uMaterialColor.rgb, uMetalness); // 基于金属度插值
	return F0 + (1 - F0) * pow(1.f - NdotV, 5);
}

// 计算 wo 方向的 exsiting radiance
vec3 BRDF(vec3 normal, vec3 posToCamera, vec3 posToLight) {
	vec3 halfVector = normalize(posToLight + posToCamera);
	float NdotH = max(dot(normal, halfVector), 0.f);
	float NdotV = max(dot(normal, posToCamera), 0.f);
	float NdotL = max(dot(normal, posToLight), 0.f);

	float D = NDF_GGXTR(NdotH);
	vec3 F = Fresnel_Schlick(NdotV);
	float G = G_Smith(NdotV, NdotL);

	vec3 fCookTorrance = (D * F * G) / (4.f * NdotV * NdotL + 0.0001);

	vec3 ks = F;
	vec3 kd = 1.f - ks;
	kd *= 1.0 - uMetalness;
	vec3 fLambert = uAlbedo / pi; // 根据金属度调整漫反射率
	return kd * fLambert + ks * fCookTorrance;
}

void main() {
	vec3 normal = normalize(fIn.normal);
	vec3 posToCamera = normalize(uCameraPosition - fIn.pos);
	vec3 ambientColor = vec3(0.03) * uAlbedo * (1.f - uAO);
	
	vec3 pLo = vec3(0.f);
	for(int i = 0; i < 4; ++i) {
		vec3 posToLight = uLightPositions[i] - fIn.pos;
		float lenPosToLight = length( posToLight);
		float attenuation = 1.f / (1.f + 0.09 * lenPosToLight + 0.032 * lenPosToLight* lenPosToLight); // 一个简单的衰减
		posToLight = normalize(posToLight);

		vec3 incidentRadiance =  uLightColors[i] * attenuation;
		pLo += BRDF(normal, posToCamera, posToLight) * incidentRadiance * max(dot(normal, posToLight), 0.f);
	}
	vec3 resultColor = pLo + ambientColor;
	resultColor = resultColor / (resultColor + vec3(1.0)); // 色调映射
	resultColor = pow(resultColor, vec3(1 / 2.2f)); // Gamma 校正
	fColor = vec4(resultColor, 1.f);
}