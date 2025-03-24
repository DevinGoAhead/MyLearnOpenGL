#version 330 core

struct GBuffer {
	sampler2D texturePosition;
	sampler2D textureNormal;
};

out float fOccusion;
in vec2 vTexCoords;

uniform int uWndWidth;
uniform int uWndheight;
uniform vec3 uSamples[64];
uniform mat4 uProjection;
uniform float uRadius; // 动态调整采样点范围
uniform float uBias; // 动态调整遮蔽灵敏度

uniform sampler2D uTextureNoise;
uniform GBuffer uGBuffer;

void main() {
	// 一个对齐观察空间(法线方向)的随机旋转的 TBN
	vec2 noiseScale = vec2(uWndWidth / 5.f, uWndheight / 5.f); // 纹理重复平铺
	vec3 noiseVec = texture(uTextureNoise, vTexCoords * noiseScale).xyz;
	vec3 normal = texture(uGBuffer.textureNormal, vTexCoords).xyz;
	vec3 tangent = normalize(noiseVec - normal * dot(normal, noiseVec));
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN = mat3(tangent, bitangent, normal);

	vec3 pos = texture(uGBuffer.texturePosition, vTexCoords).xyz; // 当前点, 也就是 kernel 的球心
	float occusion = 0.f;
	for(int i = 0; i < 64; ++i) {
		vec3 samplePos = pos + uSamples[i] * uRadius; // 采样点在观察空间中的位置
		
		// 将 samplePos 投影到屏幕空间
		vec4 samplePosScreen = uProjection * vec4(samplePos, 1.f); // 裁剪空间
		//透视除法, 但其实没有严格在 NDC 中, 因为没有做任何裁剪操作, 所以立方体的边长可能 > 2, 不过还好, 纹理环绕方式是 clap to edge
		samplePosScreen /= samplePosScreen.w;
		samplePosScreen = (samplePosScreen + 1.f) / 2.f; // 转换到 [0, 1], 但实际可能比这个范围大

		float zSample = samplePos.z; //随机采样点的 z 坐标
		float zGbuffer = texture(uGBuffer.texturePosition, samplePosScreen.xy).z; //采样点对应的屏幕像素, 对应的gBUffer 中存储的实际的 positions 的 z 值

		// 采样点投影到屏幕后正好对应到一个前方的物体的边缘上, 即使这个物体在核心点很远的范围, 也会误判遮挡
		//   采样到背景区域也 *可能* 有这样的问题
		//   这里引入 smoothstep, Returns 0.0 if x ≤ edge0 and 1.0 if x ≥ edge1, 并在[0, 1]之间插值
		//   将遮挡因子与 z 插值之间建立映射关系, 当 z 值相差越大, rangeCheck越接近0
		// 我感觉没什么必要引入 abs, 如果是后方的边缘, zGbuffer - zSample < 0, rangeCheck = 0, 不会判断遮挡, 与实际是相符的
		float rangeCheck = smoothstep(0.f, 1.f, uRadius / abs(zGbuffer - zSample)); 
		
		if(zGbuffer > zSample + uBias) occusion += 1.f; ; // occusion += 1.f * rangeCheck 
	}

	fOccusion = occusion / 64.f;
}