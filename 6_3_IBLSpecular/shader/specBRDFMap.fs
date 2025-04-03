#version 330 core

in vec2 vTexCoords;
out vec2 fColor;

float PI = 3.14159265358979;

//生成 Van Der Corput 低差异值, 生成的值是均匀的
float RadicalInverse_VdC(uint bits) {
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

vec2 Hammersley(uint i, uint Nr) {	
	// 低差异序列的坐标, 第一维均匀分布, 第二维是 Van Der Corput 低差异值
	return vec2(float(i)/float(Nr), RadicalInverse_VdC(i));
}

// 重要性采样
// xi, ξ, 低差异序列
vec3 ImportanceSampleGGX(vec2 xi, float alpha) {
	// 生成符合 pdf(ωh) 分布的样本: 极角
	// CDF(ωh) 逆变换采样, CDF(x) 中的自变量为cosθ, 而不是θ
	float cosTheta = sqrt((1.f - xi.y) / ((alpha * alpha - 1.f) * xi.y + 1.f));
	float theta = acos(cosTheta); // 采样 H 向量在球坐标中的极角

	float phi = 2 * PI * xi.x; // phi 是各项同性的, 因此可以均匀采样
	
	//H, 切线空间
	float sinTheta = sin(theta);

	// 将采样点转换到世界空间
	return vec3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta); // 在半球上的坐标
	// BRDF 的预计算是在切线空间中完成的, 因此不需要转换到世界空间
}

float G_SchlickGGX(float Ndot, float k) {
	float numer = Ndot; // 分子
	// 分母
	float denom = numer * (1.f - k) + k;
	return numer / denom;
}

float G_Smith(float NdotV, float NdotL, float alpha) {
	//float k = alpha * alpha * 0.5;
	float k = alpha * 0.5;// 效果更好
	return  G_SchlickGGX(NdotV, k) * G_SchlickGGX(NdotL, k);
}

vec2 IntegrateBRDF(float NdotV, float roughness) {
	vec3 N = vec3(0.f, 0.f, 1.f); // 切线空间, 假设法线总是朝向正 z 轴方向
	
	// 由于 BRDF 的各项同性, V 的方位角不会影响 BRDF 的预计算, 这里假设 V 在切线空间的 YZ 平面
	vec3 V;
	V.x = 0;
	V.y = sqrt(1 - NdotV * NdotV); // 勾股定理
	//V.y = 0.f;
	V.z = NdotV; // V 在 N 上的投影就是 V 的 z 坐标


	float alpha = roughness * roughness;
	uint nrSamples = 1024u;
	float A = 0.f; // F0 系数
	float B = 0.f; // F0 偏移项
	for(uint i = 0u; i < nrSamples; ++i) {
		vec2 xi = Hammersley(i, nrSamples);
		vec3 H = ImportanceSampleGGX(xi, alpha); // 切线空间, 微表面法线

		float VdotH = max(dot(V, H), 0.f);
		vec3 L = 2 * VdotH * H - V;

		float NdotL = L.z;
		
		if(NdotL > 0.f) {
			float Fterm = pow((1 - VdotH), 5);
			float NdotH = max(H.z, 0.f);

			float G = G_Smith(NdotV, NdotL, alpha);
			float Gvis = (G * VdotH) / (NdotH * NdotV);
			A += (1 - Fterm) * Gvis;
			B += Fterm * Gvis;
		}
	}
	return vec2(A /=  nrSamples, B /= nrSamples);
}
void main() {
	fColor = IntegrateBRDF(vTexCoords.x, vTexCoords.y); // 所有上半球内 V 的极角θ 和 模型表面粗糙度的组合
	//fColor = vec2(0.9, 0.0);
}