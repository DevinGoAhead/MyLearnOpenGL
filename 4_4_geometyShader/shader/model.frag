#version 330 core

struct Material{
	sampler2D textureDiffuse0;
	sampler2D textureSpecular0;
	int shininess;
};

struct Light{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	vec3 pos;
};

uniform Material material_;
uniform Light light_;
uniform vec3 cameraPos_;

out vec4 _fragColor_; 
in vec2 _texCoords_;
in mat4 _inverViewProj_; // 投影视图变换的逆矩阵
in vec3 _position_; // 裁剪空间中的点
in vec3 _normal_; // 世界空间中的法线


void main(){
	vec3 position = vec3(_inverViewProj_ * vec4(_position_, 1.f)); // 转换到世界空间
	vec3 normal = normalize(_normal_);
	vec3 point2light = normalize(light_.pos - position);
	vec3 point2camera = normalize(cameraPos_ - position);
	vec3 refelctLight = reflect(-point2light, normal);

	vec3 ambientColor = vec3(texture(material_.textureDiffuse0, _texCoords_)) * light_.ambient;
	
	float diff = max(dot(point2light, normal), 0.f);
	vec3 diffuseColor = vec3(texture(material_.textureDiffuse0, _texCoords_)) * light_.diffuse * diff;

	float spec = pow(max(dot(point2camera, refelctLight), 0.f), material_.shininess);
	vec3 specularColor = vec3(texture(material_.textureSpecular0, _texCoords_)) * light_.specular * spec;

	_fragColor_ = vec4(ambientColor + diffuseColor + specularColor, 1.f);

}