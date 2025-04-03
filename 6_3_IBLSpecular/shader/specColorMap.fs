#version 330 core

in vec3 vPosWd;
out vec3 fColor;
uniform float uRoughness;
uniform samplerCube uTextureCube;
uniform int uResolution; // envCubeTex 的边长(长宽相等)

float PI = 3.14159265358979;
float Epsilon = 0.00001;

//生成 Van Der Corput 低差异值, 生成的值是均匀的
float RadicalInverse_VdC(uint bits) {
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

vec2 Hammersley(uint i, uint N) {	
	// 低差异序列的坐标, 第一维均匀分布, 第二维是 Van Der Corput 低差异值
	return vec2(float(i)/float(N), RadicalInverse_VdC(i));
}

// 重要性采样
// xi, ξ, 低差异序列
// 今天确定了一件事, 对常规数据, 在 GPU 一次计算, 多次拷贝的性能要优于0 拷贝, 多次计算,比如下面的 N
vec3 ImportanceSampleGGX(vec2 xi, float alpha, vec3 N) {
	// 生成符合 pdf(ωh) 分布的样本: 极角
	// CDF(ωh) 逆变换采样, CDF(x) 中的自变量为cosθ, 而不是θ
	float cosTheta = sqrt((1.f - xi.y) / ((alpha * alpha - 1.f) * xi.y + 1.f));
	float theta = acos(cosTheta); // 采样 H 向量在球坐标中的极角

	float phi = 2 * PI * xi.x; // phi 是各项同性的, 因此可以均匀采样
	
	//H, 切线空间
	float sinTheta = sin(theta);
	vec3 H = vec3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta); // 在半球上的坐标

	// 为当前点构造基于世界坐标系的 TBN
	// 避免出现  N = vec3(1.f, 0.f, 0.f), T 也= vec3(1.f, 0.f, 0.f), 导致后面正交化失败
	vec3 T = (abs(N.x) < 0.999f ? vec3(1.f, 0.f, 0.f) : vec3(0.f, 0.f, 1.f));
	T = normalize(N - N * dot(N, T));
	vec3 B = cross(N, T);

	// 将采样点转换到世界空间
	return mat3(T, B, N) * H; // 不必归一化
}

float D_GGXTR(float NdotH, float alpha) {
	float numer = alpha * alpha; // 分子

	// 分母
	float denom = (NdotH * NdotH) * (numer * numer - 1.f) + 1.f;
	denom = PI * denom * denom;
	return numer / (denom + Epsilon);
}

float GetMipLevel(float alpha, float NdotH, float OdotH, uint nrSamples) {
	float D = D_GGXTR(NdotH, alpha);
	float pdf = (D * NdotH) / (4 * OdotH + Epsilon); // 当前采样方向的 pddf 值
	float texelWeightToSdAg = 4 * PI / (6 * uResolution * uResolution); // 6个面, 覆盖 4π 球面度, 单个纹素覆盖的立体角范围
	float sampleWeightToSdAg = 1.f / (nrSamples * (pdf + Epsilon)); // 单个样本覆盖的立体角范围
	return uRoughness == 0.f ? 0 : 0.5f * log2(sampleWeightToSdAg / texelWeightToSdAg);
}

void main() {
	vec3 N = normalize(vPosWd);
	vec3 V = N; // 假设视线方向与法线同向
	vec3 O = V; // 因为是镜面反射, 出射(反射)方向与视线同向,  否则无法被看到
	float alpha = uRoughness * uRoughness;
	uint nrSamples = 1024u;
	vec3 preFilterColor = vec3(0.f);
	float weight = Epsilon;
	float mipLevel = 0.f;
	for(uint i = 0u; i < nrSamples; ++i) {
		vec2 xi = Hammersley(i, nrSamples); //ξ
		vec3 H = ImportanceSampleGGX(xi, alpha, N);
		vec3 L = 2 * dot(H, V) * H - V; // V + L = 2 * dot(H, V)
		float NdotH = max(dot(N, H), 0.f);
		float OdotH = max(dot(O, H), 0.f);
		float mipLevel = GetMipLevel(alpha, NdotH, OdotH, nrSamples);
		float NdotL = dot(N, L);
		// 完全看不懂 Σ((L(l_i) *  N·l_i) / Σ(N·l_i)) 是在做什么, 完全不合理, 物理的数学的都不合理
		if(NdotL > 0) { // 避免采样到下半球
			preFilterColor += textureLod(uTextureCube, L, mipLevel).rgb * NdotL;
			weight += NdotL;
		}
	}
	// fColor = vec3(0.f, mipLevel / 4.f, 0.f);
	fColor = preFilterColor / weight;
}