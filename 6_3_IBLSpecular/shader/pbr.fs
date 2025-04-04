#version 330 core

in VOUT {
	vec3 pos;
	vec3 normal;
	vec2 texCoords;
} fIn;

out vec4 fColor;

uniform vec3 uF0; // 基础反射率
uniform samplerCube uTextureDiffE;
uniform samplerCube uTextureSpecColor;
uniform sampler2D uTextureSpecBRDF;
uniform int uMaxMipLevel;
uniform sampler2D uTextureAlbedo;
uniform sampler2D uTextureNormal;
uniform sampler2D uTextureMetallicity;
uniform sampler2D uTextureRoughness;
uniform sampler2D uTextureAO;

uniform vec3 uCameraPosition;
uniform vec3 uLightPositions[4];
uniform vec3 uLightColors[4];

float pi = 3.14159265359;

vec3 albedo = texture(uTextureAlbedo, fIn.texCoords).rgb;
float metalness = texture(uTextureMetallicity, fIn.texCoords).x;
float roughness = texture(uTextureRoughness, fIn.texCoords).x;// 粗糙度
float AO = texture(uTextureAO, fIn.texCoords).x;

float NDF_GGXTR(float NdotH) {
	//float a = roughness * roughness;
	float a = roughness; // 无明显差异
	float numer = a * a; // 分子

	// 分母
	float denom = (NdotH * NdotH) * (numer * numer - 1.f) + 1.f;
	denom = pi * denom * denom;
	return numer / (denom + 0.0001);
}

float G_SchlickGGX(float Ndot, float k) {
	float numer = Ndot; // 分子

	// 分母
	float denom = numer * (1.f - k) + k;
	return numer / denom;
}

float G_Smith(float NdotV, float NdotL) {
	//float alpha = roughness * roughness;
	float alpha = roughness;// 无明显差异
	float k = (alpha + 1.f) * (alpha + 1.f) * 0.125;
	return  G_SchlickGGX(NdotV, k) * G_SchlickGGX(NdotL, k);
}

vec3 Fresnel_Schlick(float HdotV) {
	vec3 F0 = mix(uF0, albedo, metalness); // 基于金属度插值
	return F0 + (1 - F0) * pow(1.f - HdotV, 5);
}

vec3 Fresnel_Schlick_Roughness(float HdotV) {
	vec3 F0 = mix(uF0, albedo, metalness);
	return F0 + (max(vec3(1.f - roughness), F0) - F0) * pow(1.f - HdotV, 5);
}

// 计算 wo 方向的 exsiting radiance
vec3 BRDF(vec3 normal, vec3 posToCamera, vec3 posToLight) {
	vec3 halfVector = normalize(posToLight + posToCamera);
	float NdotH = max(dot(normal, halfVector), 0.f);
	float NdotV = max(dot(normal, posToCamera), 0.f);
	float NdotL = max(dot(normal, posToLight), 0.f);
	float HdotV = max(dot(posToCamera, halfVector), 0.f);

	float D = NDF_GGXTR(NdotH);
	vec3 F = Fresnel_Schlick_Roughness(HdotV);
	float G = G_Smith(NdotV, NdotL);

	vec3 fCookTorrance = (D * F * G) / (4.f * NdotV * NdotL + 0.0001);

	vec3 ks = F;
	vec3 kd = 1.f - ks;
	kd *= (1.0 - metalness);
	vec3 fLambert = albedo / pi; // 根据金属度调整漫反射率
	return kd * fLambert + ks * fCookTorrance;
}

vec3 TangentToWorld() {
 	vec3 normalTan = 2.f * texture(uTextureNormal, fIn.texCoords).xyz - 1.f; // 转换到[-1, 1]
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

	vec3 ks = Fresnel_Schlick_Roughness(max(dot(normal, posToCamera), 0.f)); // 由于没有确定的半程向量, 所以这里使用法线和视线夹角
	vec3 kd = 1.f - ks;
	kd *= (1.0 - metalness);

	//specularColor
	float NdotV = max(dot(normal, posToCamera), 0);
	vec3 F0 = mix(uF0, albedo, metalness); // 基础反射率, 基于金属度插值
	vec2 lut = texture(uTextureSpecBRDF, vec2(NdotV, roughness)).xy;

	// 示例代码中这里使用的F, 即ks, 但是根据 BRDF 预计算公式推导, 这里应使用F0
	// 实际测试并不能看出有明显的差异
	vec3 brdf =  ks * lut.x + lut.y;
	vec3 L = 2 * dot(normal, posToCamera) * normal - posToCamera;
	float mipLevel = roughness * uMaxMipLevel;
	vec3 specularColor = brdf * textureLod(uTextureSpecColor, L, mipLevel).rgb;
	//vec3 specularColor = brdf * vec3(1.0);//debug
	
	// diffuseColor 
	// 因为 IBL 中假设所有 irradiance 均匀分布在场景中心
	// 也就是无论物体在什么位值, 计算其 radiance 时假设所有物体都在场景中心, 其接受到的 irradiance 与位值无关
	// 所以这里还是取用片断的法线, 然后将其放在立方体中心, 采样法线方向的 irradiance
	vec3 diffuseColor = kd * texture(uTextureDiffE, normal).rgb * albedo; 
	
	vec3 ambientColor = (diffuseColor + specularColor) * AO;
	//vec3 ambientColor = (specularColor) * AO;
	vec3 resultColor = pLo * 0.3f + ambientColor; // 这里 pLo 暗一些, 效果更好
	//resultColor = pLo;
	resultColor = resultColor / (resultColor + vec3(1.0)); // 色调映射
	resultColor = pow(resultColor, vec3(1 / 2.2f)); // Gamma 校正
	fColor = vec4(resultColor, 1.f);
	//fColor = vec4(vec3(metalness), 1.f);
}