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
	float depth = texture(uMaterial.textureHeight, fIn.texCoord).r; // 当前点采样深度值
	// 以下 两行代码是后加的, 否则砖缝扭曲很严重
	// 
	depth = depth * 2.0 - 1.0;  // 将 [0,1] 映射到 [-1,1][凸起, 凹陷]
	depth *= 0.1;           // 乘上一个适当的缩放因子
	// viewDir 在UV平面的分量控制 offset 的方向, depth 控制偏移值
	// uScale控制 为偏移提供一个固定的全局的比例系数(缩小), point2Camera.z 则会根据视角倾斜角度控制偏移的倍乘系数(扩大)
	vec2 offSet = point2Camera.xy * depth * uScale / point2Camera.z;
	return fIn.texCoord - offSet;
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