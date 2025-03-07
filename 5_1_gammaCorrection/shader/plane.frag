#version 330 core

struct Material {
	sampler2D textureDiffuse;
	sampler2D textureSpecular;
	int shininess;
};

struct Light {
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	vec3 pos;
};

in vec3 vPos;
in vec3 vNormal;
in vec2 vTexCoord;
out vec4 fColor;

uniform vec3 uCameraPos;
uniform Material uMaterial;
uniform Light uLight;
uniform bool uBlinn;

void main() {
	vec3 normal = normalize(vNormal);
	vec3 point2Light = normalize(vPos - uLight.pos);
	vec3 point2Camera = normalize(vPos - uCameraPos);
	vec3 reflectLight = reflect(-point2Light, normal);
	vec3 bisector = normalize(point2Light + point2Camera);

	vec3 colorDiffuse = texture(uMaterial.textureDiffuse, vTexCoord).rgb;

	vec3 resultAmbient = colorDiffuse* uLight.ambient;

	float diff = max(dot(point2Light, normal), 0);
	vec3 resultDiffuse = colorDiffuse * uLight.diffuse * diff;
	
	float spec = 0.f;
	if(uBlinn) {spec = pow(max(dot(bisector, normal), 0), 32);}
	else {spec = pow(max(dot(point2Camera, reflectLight), 0), 32);}

	vec3 resultSpecular = vec3(0.3f) * spec; // 没有反射纹理, 假设一个反射颜色
	//vec3 resultSpecular = texture(uMaterial.textureSpecular, vTexCoord).rgb * uLight.specular * spec;

	fColor = vec4(resultAmbient + resultDiffuse + resultSpecular, 1.f);
}