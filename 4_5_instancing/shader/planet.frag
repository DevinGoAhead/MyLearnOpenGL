#version 330 core

struct Material {
	sampler2D textureDiffuse0;
	sampler2D textureSpecular0;
	uint shininess;
};

struct Light{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	vec3 pos;
};

in vec2 vTexCoord;
in vec3 vPos;
in vec3 vNormal;
out vec4 fColor;

uniform Material uMaterial;
uniform Light uLight;
uniform vec3 uCameraPos;

void main() {
	vec3 normal = normalize(vNormal);
	vec3 point2Light = normalize(uLight.pos - vPos);
	vec3 point2Camera = normalize(uCameraPos - vPos);
	vec3 bisector = normalize(point2Light + point2Camera);

	vec3 resultAmbi = texture(uMaterial.textureDiffuse0, vTexCoord).rgb * uLight.ambient;
	
	float diff = max(dot(point2Light, normal), 0.f);
	vec3 resultDiff = texture(uMaterial.textureDiffuse0, vTexCoord).rgb * uLight.diffuse * diff;

	float spec = pow(max(dot(bisector, normal), 0.f), uMaterial.shininess);
	vec3 resultSpec = texture(uMaterial.textureSpecular0, vTexCoord).rgb * uLight.specular * spec;

	fColor = vec4(resultAmbi + resultDiff + resultSpec, 1.f);
	//fColor = vec4(normal, 1.f);
	
}