#version 330 core

in VOUT {
	vec3 pos;
	vec3 normal;
	vec2 texCoords;
} fIn;

out vec4 fColor;

uniform sampler2D uRoughness; // 粗糙度
uniform vec3 uF0; // 基础反射率
uniform sampler2D uMetalness; // 金属度
uniform sampler2D uAlbedo; // 反照率
uniform sampler2D uAO; // 环境光遮蔽系数
uniform sampler2D uNormal; // 法线

uniform vec3 uCameraPosition;
uniform vec3 uLightPositions[4];
uniform vec3 uLightColors[4];

float pi = 3.14159265359;

float roughness = texture(uRoughness, fIn.texCoords).r;
float metalness = texture(uMetalness, fIn.texCoords).r;
vec3 albedo = texture(uAlbedo, fIn.texCoords).rgb;

float NDF_GGXTR(float NdotH) {
	float a = roughness * roughness; // 分子
	float numer = roughness * roughness; // 分子

	// 分母
	float denom = (NdotH * NdotH) * (numer * numer - 1.f) + 1.f;
	denom = pi * denom * denom;
	return (numer * numer) / (denom + 0.0001);
}

float G_SchlickGGX(float Ndot, float k) {
	float numer = Ndot; // 分子

	// 分母
	float denom = numer * (1.f - k) + k;
	return numer / denom;
}

float G_Smith(float NdotV, float NdotL) {
	float k = (roughness + 1.f) * (roughness + 1.f) * 0.125;
	return  G_SchlickGGX(NdotV, k) * G_SchlickGGX(NdotL, k);
}

vec3 Fresnel_Schlick(float HdotV) {
	vec3 F0 = mix(uF0, albedo, metalness); // 基于金属度插值
	return F0 + (1 - F0) * pow(1.f - HdotV, 5);
}

// 计算 wo 方向的 exsiting radiance
vec3 BRDF(vec3 normal, vec3 posToCamera, vec3 posToLight) {
	vec3 halfVector = normalize(posToLight + posToCamera);
	float NdotH = max(dot(normal, halfVector), 0.f);
	float NdotV = max(dot(normal, posToCamera), 0.f);
	float NdotL = max(dot(normal, posToLight), 0.f);
	float HdotV = max(dot(posToCamera, halfVector), 0.f);

	float D = NDF_GGXTR(NdotH);
	vec3 F = Fresnel_Schlick(HdotV);
	float G = G_Smith(NdotV, NdotL);

	vec3 fCookTorrance = (D * F * G) / (4.f * NdotV * NdotL + 0.0001);

	vec3 ks = F;
	vec3 kd = 1.f - ks;
	kd *= 1.0 - metalness;
	vec3 fLambert = albedo / pi; // 根据金属度调整漫反射率
	return kd * fLambert + ks * fCookTorrance;
}

vec3 TangentToWorld() {
	vec3 normalTan = 2.f * texture(uNormal, fIn.texCoords).xyz - 1.f; // 转换到[-1, 1]
	vec3 normalWd = normalize(fIn.normal);

	vec3 E1 = dFdx(fIn.pos);
	vec3 E2 = dFdy(fIn.pos);
	vec2 dUV1 = dFdx(fIn.texCoords);
	vec2 dUV2 = dFdy(fIn.texCoords);

	// 求解过程与 法线贴图那里是一样的
	// 代数余子式求解逆矩阵, 归一化不需要考虑行列式分子, 详见 onenote 笔记
	vec3 T = E1 * dUV2.y - E2 * dUV1.y;
	T = normalize(T - normalWd * dot(normalWd, T));
	vec3 B = -cross(normalWd, T);
	return mat3(T, B, normalWd) * normalTan;
}

void main() {
	vec3 normal = TangentToWorld();
	vec3 posToCamera = normalize(uCameraPosition - fIn.pos);

	float ao = texture(uAO, fIn.texCoords).r;
	vec3 ambientColor = vec3(0.03) * albedo * ao; // ao, 而不是 1 - ao
	
	vec3 pLo = vec3(0.f);
	for(int i = 0; i < 4; ++i) {
		vec3 posToLight = uLightPositions[i] - fIn.pos;
		float lenPosToLight = length( posToLight);
		//float attenuation = 1.f / (1.f + 0.7 * lenPosToLight + 1.8f * lenPosToLight* lenPosToLight); // 一个简单的衰减
		float attenuation = 1.f / (lenPosToLight * lenPosToLight); // 测试示例效果
		posToLight = normalize(posToLight);

		vec3 incidentRadiance =  uLightColors[i] * attenuation;
		pLo += BRDF(normal, posToCamera, posToLight) * incidentRadiance * max(dot(normal, posToLight), 0.f);
	}
	vec3 resultColor = pLo + ambientColor;
	resultColor = resultColor / (resultColor + vec3(1.0)); // 色调映射
	resultColor = pow(resultColor, vec3(1 / 2.2f)); // Gamma 校正
	fColor = vec4(resultColor, 1.f);
}