#version 330 core

struct Material {
	sampler2D textureDiffuse0;
	sampler2D textureSpecular0;
	int shininess;
};

in VOUT {
	vec3 posWdSpace;
	vec3 normal;
	vec2 texCoord;
	vec4 posClipSpaceAtLgt; // "光" 坐标系下的裁减空间, 即 Proj 变换后, 未裁减, 未透视除法
} fIn;
out vec4 fColor;

uniform sampler2D uTextureDepthMap;
uniform vec3 uCameraPos;
uniform vec3 uLightPos;
uniform Material uMaterial;
// uniform int uIs3D; // 标识模型是否为 3D(不需要展示内部的3D), 因为实测 Shadow Bias 效果比 Cull Front 效果更好, 所以舍弃这个方案

// true: in shadow
float DepthTest(vec3 point2Light, vec3 normal) {
	vec4 posPerspDived = fIn.posClipSpaceAtLgt / fIn.posClipSpaceAtLgt.w;
	vec3 posPerspDived2Tex = (posPerspDived.xyz + 1.f) * 0.5;
	float zPerspDived2Tex = posPerspDived2Tex.z;
	// 因为 光空间的点没有通过glPosition传递, 因此图形管线不会对这些点进行裁剪
	// 因此 xyz 都可能超过1.0, 这里解决 z > 1.0 的情况, 直接设定为被照亮
	// 但是实际测试下来 Clap to edge + if(zPerspDived2Tex > 1.f) return false; 就可以照亮整个场景了, 不知是否合理
	if(zPerspDived2Tex > 1.f) return 0.f;
	
	// (1 - max(dot(point2Light, normal), 0.f)), 夹角越大, 值越大, 范围[0, 1]
	// valueMax * (1 - max(dot(point2Light, normal), 0.f)), 得[0, valueMax]
	// 最外围的max, [bias, 0.05]
	float bias = max(0.18 * (1 - max(dot(point2Light, normal), 0.f)), 0.005);

	// PCF
	// textureSize 一个指定采样器对应的第 0 级MipMap纹理的尺寸
	// 1 /, 则得到单个纹素的尺寸
	vec2 texelSize = 1.f / textureSize(uTextureDepthMap, 0);
	float percentUnshadow = 0.f;
	for(int i = -1; i < 2; ++i) {
		for(int j = -1; j < 2; ++j){
			float zDepthMap = texture(uTextureDepthMap, posPerspDived2Tex.xy + vec2(i, j) * texelSize).r;
			if(zPerspDived2Tex < zDepthMap + bias) // 不在阴影中
				percentUnshadow += 1.f;
		}
	}
	return percentUnshadow / 9.f;
}

void main() {
	vec3 lightColor = vec3(0.3); // 这里直接将光照强度硬编码在着色器中
	vec3 materialColor = texture(uMaterial.textureDiffuse0, fIn.texCoord).rgb; // 本例仅有漫反射纹理

	vec3 normal = normalize(fIn.normal);
	vec3 point2Camera = normalize(uCameraPos - fIn.posWdSpace);
	vec3 point2Light = normalize(uLightPos - fIn.posWdSpace);
	vec3 bisector = normalize(point2Light + point2Camera);

	vec3 lightAmbient = lightColor * 0.3; // 不进行阴影计算, 避免全黑
	
	float percent = DepthTest(point2Light, normal);
	float diff = max(dot(point2Light, normal), 0.f);
	vec3 lightDiffuse = lightColor * diff * percent;

	float spec = pow(max(dot(bisector, normal), 0.f), uMaterial.shininess);
	vec3 lightSpecular =lightColor * spec * percent;

	vec3 color = (lightAmbient + lightDiffuse + lightSpecular) * materialColor;
	color = pow(color, vec3(1.f / 2.2)); // gamma correction
	fColor = vec4(color, 1.f);
}