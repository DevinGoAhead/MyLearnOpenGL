#version 330 core

struct Material
{
	sampler2D textureDiffuse0;
	sampler2D textureSpecular0;
	sampler2D textureReflection0;
	float shiness;
};

//平行光
struct DirecLight
{
	vec3 ambientColor;
	vec3 diffuseColor;
	vec3 specularColor;
	vec3 direc;
};

uniform vec3 cameraPos_;
uniform Material material_;
uniform DirecLight direcLight_;
uniform samplerCube texerSkyBox_;

in vec3 _vertex_;
in vec3 _normal_;
in vec2 _texCoords_;
out vec4 _fragColor_;

void main() {
	vec3 normal = normalize(_normal_);
	vec3 point2camera = normalize(cameraPos_ - _vertex_);
	vec3 cameraRelect = reflect(-point2camera, normal);

	vec3 point2light = -direcLight_.direc;
	vec3 lightRelect = reflect(-point2light, normal);

	// 环境光项
	vec3 ambient = texture(material_.textureDiffuse0, _texCoords_).rgb * direcLight_.ambientColor;

	// 漫反射项
	float diff = max(dot(normal, point2light), 0.0);
	vec3 diffuse = texture(material_.textureDiffuse0, _texCoords_).rgb * direcLight_.diffuseColor * diff;

	// 高光项
	float spec = pow(max(dot(point2camera, lightRelect), 0.0f), material_.shiness);
	vec3 specular = texture(material_.textureSpecular0, _texCoords_).rgb * direcLight_.specularColor * spec;

	// 反射光项
	float reflect = max(dot(normal, point2camera), 0.0);
	vec3 texReflection = texture(material_.textureReflection0, _texCoords_).rgb;

	// skyBox
	vec3 envReflection = texture(texerSkyBox_, cameraRelect).rgb;

	// 综合
	_fragColor_ = vec4(ambient + diffuse + specular + texReflection * envReflection, 1.f);
	//_fragColor_ = vec4(ambient + diffuse + specular, 1.f);

}