#version 330 core

in vec3 vPos;
in vec3 vNormal;
in vec2 vTexCoord;
out vec4 fColor;

uniform sampler2D textureDiffuse;
uniform vec3 uCameraPos;
uniform vec3 uLightColors[4];
uniform vec3 uLightPoses[4];
uniform bool uGamma;

vec3 normal = normalize(vNormal);
vec3 point2Camera = normalize(uCameraPos - vPos);

// 一个简单的场景
vec3 CalBlinnPhongLightColor(vec3 lightPos, vec3 lightColor) {
	float distancePtLigt = length(lightPos - vPos);
	vec3 point2Light = normalize(lightPos - vPos);
	vec3 bisector = normalize(point2Light + point2Camera);

	float diff = max(dot(point2Light, normal), 0);
	vec3 lightDiffuse = lightColor * diff;
	
	float spec = pow(max(dot(bisector, normal), 0), 64);
	vec3 lightSpecular = lightColor * spec;

	// 衰减
	float attenuation;
	if(uGamma) attenuation = 1.f / (distancePtLigt * distancePtLigt + 1.f);// 1 /2.2 , 矫正硬件的 2.2次幂
	else attenuation = 1.f / (distancePtLigt + 1.f); // 硬件 2.2次幂会放大衰减效果

	return (lightDiffuse + lightSpecular) * attenuation;
}

void main() {
	vec3 lightColors = vec3(0.f);
	for(int i = 0; i < 4; ++i) {
		lightColors += CalBlinnPhongLightColor(uLightPoses[i], uLightColors[i]);
	}

	vec3 color= texture(textureDiffuse, vTexCoord).rgb; // 本例仅有漫反射纹理
	color *= lightColors;
	if(uGamma) color = pow(color, vec3(1.f / 2.2));
	fColor = vec4(color, 1.f);
}