#version 330 core

struct Material {
	sampler2D textureDiffuse0;
	sampler2D textureNormal;
};

in VOUT {
	vec3 pos;
	vec3 normal;
	vec2 texCoord;
} fIn;

out vec4 fColor;

uniform Material uMaterial;
uniform vec3 uLightPos;
uniform vec3 uCameraPos;

void main() {
	vec3 normal = normalize(fIn.normal);
	vec3 lightColor = vec3(0.8); // 这里直接将光照强度硬编码在着色器中
	vec3 materialColor = texture(uMaterial.textureDiffuse0, fIn.texCoord).rgb; // 本例仅有漫反射纹理
	
	vec3 point2Camera = normalize(uCameraPos - fIn.pos);
	vec3 point2Light = normalize(uLightPos - fIn.pos);
	vec3 bisector = normalize(point2Light + point2Camera);

	vec3 lightAmbient = lightColor * 0.3; // 不进行阴影计算, 避免全黑
	
	float diff = max(dot(point2Light, normal), 0.f);
	vec3 lightDiffuse = lightColor * diff;

	float spec = pow(max(dot(bisector, normal), 0.f), 8);
	vec3 lightSpecular =lightColor * spec;

	vec3 color = (lightAmbient + lightDiffuse + lightSpecular) * materialColor;
	color = pow(color, vec3(1.f / 2.2)); // gamma correction
 	
	fColor = vec4(color, 1.f);
}