#version 330 core

struct Material {
	sampler2D textureDiffuse0;
	sampler2D textureSpecular0;
	sampler2D textureNormal0;
};

in VOUT {
	vec3 posTan; //切线空间
	vec3 lightPosTan; // 切线空间
	vec3 cameraPosTan; // 切线空间
	vec2 texCoord;
} fIn;

out vec4 fColor;
in vec3 vNormal;
uniform Material uMaterial;

void main() {
	vec3 normal = texture(uMaterial.textureNormal0, fIn.texCoord).rgb;
	normal = (normal * 2.f) -1.f; // [-1, 1]
	//normal = normalize(vNormal);
	
	vec3 point2Camera = normalize(fIn.cameraPosTan - fIn.posTan);
	vec3 point2Light = normalize(fIn.lightPosTan - fIn.posTan);
	vec3 bisector = normalize(point2Light + point2Camera);

	vec3 Ambient = texture(uMaterial.textureDiffuse0, fIn.texCoord).rgb * 0.1;
	
	float diff = max(dot(point2Light, normal), 0.f);
	vec3 Diffuse = texture(uMaterial.textureDiffuse0, fIn.texCoord).rgb * 0.6 * diff;

	float spec = pow(max(dot(bisector, normal), 0.f), 64);
	vec3 Specular = texture(uMaterial.textureSpecular0, fIn.texCoord).rgb * 0.3 * spec;

	vec3 color = (Ambient + Diffuse + Specular);
	//vec3 color = Ambient;
	color = pow(color, vec3(1.f / 2.2)); // gamma correction
 	
	fColor = vec4(color, 1.f);
}