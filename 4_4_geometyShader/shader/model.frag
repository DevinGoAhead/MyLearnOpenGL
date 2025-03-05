#version 330 core

struct Material{
	sampler2D textureDiffuse0;
	sampler2D textureSpecular0;
	sampler2D textureReflection0;
	int shininess;
};

struct Light{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	vec3 pos;
};

uniform Material uMaterial;
uniform Light light_;
uniform vec3 cameraPos_;

out vec4 _fragColor_; 
in vec2 _texCoords_;
in vec3 _position_; // 世界空间
in vec3 _normal_; // 世界空间


void main(){
	vec3 normal = normalize(_normal_);
	vec3 point2light = normalize(light_.pos -_position_);
	vec3 point2camera = normalize(cameraPos_ - _position_);
	vec3 refelctLight = reflect(-point2light, normal);

	vec3 ambientColor = vec3(texture(uMaterial.textureDiffuse0, _texCoords_)) * light_.ambient;
	
	float diff = max(dot(point2light, normal), 0.f);
	vec3 diffuseColor = vec3(texture(uMaterial.textureDiffuse0, _texCoords_)) * light_.diffuse * diff;

	float spec = pow(max(dot(point2camera, refelctLight), 0.f), uMaterial.shininess);
	vec3 specularColor = vec3(texture(uMaterial.textureSpecular0, _texCoords_)) * light_.specular * spec;

	_fragColor_ = vec4(ambientColor + diffuseColor + specularColor, 1.f);

}