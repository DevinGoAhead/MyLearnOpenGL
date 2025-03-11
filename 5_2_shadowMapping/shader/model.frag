#version 330 core

struct Material {
	sampler2D textureDiffuse0;
	sampler2D textureSpecular0;
	int shininess;
};

in VOUT {
	vec3 pos;
	vec3 normal;
	vec2 texCoord;
} fIn;
out vec4 fColor;

uniform samplerCube uTextureDepthMap;
uniform vec3 uCameraPos;
uniform vec3 uLightPos;
uniform Material uMaterial;
// uniform int uIs3D; // 标识模型是否为 3D(不需要展示内部的3D), 因为实测 Shadow Bias 效果比 Cull Front 效果更好, 所以舍弃这个方案
//uniform int uRNormal; // Reverse Normal
uniform float uFar;

vec3 sampleOffsetDirections[20] = vec3[]
(
 vec3( 1, 1, 1), vec3( 1, -1, 1), vec3(-1, -1, 1), vec3(-1, 1, 1), 
 vec3( 1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
 vec3( 1, 1, 0), vec3( 1, -1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
 vec3( 1, 0, 1), vec3(-1, 0, 1), vec3( 1, 0, -1), vec3(-1, 0, -1),
 vec3( 0, 1, 1), vec3( 0, -1, 1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);

// float: 1.f 完全被照亮
float DepthTest() {
	vec3 light2point = fIn.pos - uLightPos; // cube纹理采样的方向向量不需要标准化
	// 或者 depthCur /uFar, 或者 depthMap * uFar, 我感觉后者精度应该更高
	float depthCur = length(light2point); // 没有被裁剪的,可能超过uFar
	
	if(depthCur > uFar) return 1.f; // 越界认为被照亮
	float diskRadius = 0.05; // 静态
	float unShadowPercent = 0.f;
	float bias = 0.05f;
	for(int i = 0; i < 20; ++i) {
		float depthMap = texture(uTextureDepthMap, light2point + sampleOffsetDirections[i] * diskRadius).r * uFar;// 深度贴图(纹理) 中存储的深度, 这是被裁剪过的,一定 < 1
		if(depthCur < depthMap + bias) unShadowPercent += 1.f;
	}
	
	return unShadowPercent / 20;
}

void main() {
	vec3 lightColor = vec3(0.3); // 这里直接将光照强度硬编码在着色器中
	vec3 materialColor = texture(uMaterial.textureDiffuse0, fIn.texCoord).rgb; // 本例仅有漫反射纹理

	vec3 normal = normalize(fIn.normal);
	//if(uRNormal == 1) normal *= -1.f;
	if(!gl_FrontFacing) normal *= -1.f;
	vec3 point2Camera = normalize(uCameraPos - fIn.pos);
	vec3 point2Light = normalize(uLightPos - fIn.pos);
	vec3 bisector = normalize(point2Light + point2Camera);

	vec3 lightAmbient = lightColor * 0.3; // 不进行阴影计算, 避免全黑
	
	float percent = DepthTest();
	float diff = max(dot(point2Light, normal), 0.f);
	vec3 lightDiffuse = lightColor * diff * percent;

	float spec = pow(max(dot(bisector, normal), 0.f), uMaterial.shininess);
	vec3 lightSpecular =lightColor * spec * percent;

	vec3 color = (lightAmbient + lightDiffuse + lightSpecular) * materialColor;
	color = pow(color, vec3(1.f / 2.2)); // gamma correction
 	
	fColor = vec4(vec3(color), 1.f);
	// float depthMap = texture(uTextureDepthMap, -point2Light).r;
	// fColor = vec4(vec3(depthMap), 1.f);
}