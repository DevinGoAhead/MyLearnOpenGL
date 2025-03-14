#version 330 core

struct Material {
	sampler2D textureDiffuse0;
	sampler2D textureNormal;
	sampler2D textureHeight;
};

in VOUT {
	vec3 posTan; //切线空间
	vec3 lightPosTan; // 切线空间
	vec3 cameraPosTan; // 切线空间
	vec2 texCoord;
} fIn;

out vec4 fColor;

uniform Material uMaterial;
uniform float uScale; //偏移比例系数

vec2 GetParaTexCoord(vec3 point2Camera) {
	float maxNumLayer = 64.f, minNumLayer = 16.f;
	// 越水平, 夹角越大, 比例值越小, 1-比例值越大,maxNumLayer占比越大
	int numLayer = int(mix(maxNumLayer, minNumLayer, abs(dot(vec3(0.f, 0.f, 1.f), point2Camera))));
	float dLayerDepth = 1.f / numLayer; //层深度递增布局
	vec2 dTexCoord = point2Camera.xy * dLayerDepth  * uScale; // 纹理递增(递减)歩距
	
	float curLayerDepth = 0.f;
	vec2 curTexCoord = fIn.texCoord, prevTexCoord = curTexCoord;
	float curSampleDepth = texture(uMaterial.textureHeight, curTexCoord).r; // 当前点采样深度值
	for(int i = 0; i < numLayer; ++i) {
		if(curSampleDepth <= curLayerDepth) {
			float prevSampleDepth = texture(uMaterial.textureHeight, prevTexCoord).r;
			float t = (curLayerDepth  - curSampleDepth) / max(abs(prevSampleDepth - (curLayerDepth - dLayerDepth)), 0.0001);// 估算, 也没什么道理
			return mix(curTexCoord, prevTexCoord, t);
		}
		curLayerDepth += dLayerDepth;
		prevTexCoord = curTexCoord;
		curSampleDepth = texture(uMaterial.textureHeight, curTexCoord -= dTexCoord).r;
	}
}

void main() {	
	vec3 point2Camera = normalize(fIn.cameraPosTan - fIn.posTan);
	vec3 point2Light = normalize(fIn.lightPosTan - fIn.posTan);
	vec3 bisector = normalize(point2Light + point2Camera);
	vec2 texCoord = GetParaTexCoord(point2Camera);
	if(texCoord.x > 1.0 || texCoord.y > 1.0 || texCoord.x < 0.0 || texCoord.y < 0.0)
		texCoord = vec2(0.f);

	vec3 lightColor = vec3(0.3); // 这里直接将光照强度硬编码在着色器中
	vec3 materialColor = texture(uMaterial.textureDiffuse0, texCoord).rgb; // 本例仅有漫反射纹理

	vec3 normal = texture(uMaterial.textureNormal, texCoord).rgb;
	normal = normalize((normal * 2.f) -1.f); // [-1, 1]
	vec3 lightAmbient = lightColor * 0.3; // 不进行阴影计算, 避免全黑
	
	float diff = max(dot(point2Light, normal), 0.f);
	vec3 lightDiffuse = lightColor * diff;

	float spec = pow(max(dot(bisector, normal), 0.f), 32);
	vec3 lightSpecular =lightColor * spec;

	vec3 color = (lightAmbient + lightDiffuse + lightSpecular) * materialColor;
	//vec3 color = (lightDiffuse) * materialColor;
	color = pow(color, vec3(1.f / 2.2)); // gamma correction
 	//float depth = texture(uMaterial.textureHeight, fIn.texCoord).r;
	fColor = vec4(color, 1.f);
}