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

// true: in shadow
bool DepthTest() {
	vec4 posPerspDived = fIn.posClipSpaceAtLgt / fIn.posClipSpaceAtLgt.w;
	vec3 posDepthMap = (posPerspDived.xyz + 1.f) * 0.5;
	float zCur = posDepthMap.z;
	float zDepthMap = texture(uTextureDepthMap, posDepthMap.xy).r;
	return zCur > zDepthMap;
}

void main() {
	vec3 lightColor = vec3(0.3); // 这里直接将光照强度硬编码在着色器中
	vec3 materialColor = texture(uMaterial.textureDiffuse0, fIn.texCoord).rgb; // 本例仅有漫反射纹理

	vec3 normal = normalize(fIn.normal);
	vec3 point2Camera = normalize(uCameraPos - fIn.posWdSpace);
	vec3 point2Light = normalize(uLightPos - fIn.posWdSpace);
	vec3 bisector = normalize(point2Light + point2Camera);

	vec3 lightAmbient = lightColor * 0.3; // 不进行阴影计算, 避免全黑
	
	float diff = max(dot(point2Light, normal), 0.f);
	float spec = pow(max(dot(bisector, normal), 0.f), uMaterial.shininess);
	vec3 lightDiffuse = vec3(0.f), lightSpecular = vec3(0.f);
	if(!DepthTest()) {
		lightDiffuse = lightColor* diff;
		lightSpecular = lightColor * spec;
	}

	vec3 color = (lightAmbient + lightDiffuse + lightSpecular) * materialColor;
	color = pow(color, vec3(1.f / 2.2)); // gamma correction
	fColor = vec4(color, 1.f);
}