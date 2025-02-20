#version 330 core

struct Material
{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shiness;
};

struct Light
{
	vec3 ambientColor;
	vec3 diffuseColor;
	vec3 specularColor;
	vec3 pos;
};

//uniform vec3 boxColor_;
uniform vec3 lightColor_;
//uniform vec3 lightPos_;
uniform vec3 cameraPos_;
//uniform sampler2D texturer0_;
uniform struct Material material_;
uniform struct Light light_;

in vec3 _normal_;
in vec3 _vertex_;
out vec4 _fragColor_;
in vec2 _verTexCoords_;

void main()
{
	//float ka_f = 0.01;
	vec3 ambientItem = material_.ambient * light_.ambientColor;
	
	vec3 point2light = normalize(light_.pos - _vertex_); //点指向光源, 单位化
	//_normal_ = normalize(_normal_); // 单位化, varing 变量, 即从外面传递过来的变量, 不能修改, 所以智能在顶点着色器中单位化
	//float kd_f = 0.8f;
	vec3 diffuseItem =  material_.diffuse * light_.diffuseColor * max(dot(point2light, _normal_), 0.f);

	vec3 point2eye = normalize(cameraPos_ - _vertex_); //点指向观察位置, 单位化
	vec3 bisector = normalize(point2eye + point2light); //半程向量
	//float ks_f = 1.f;
	//float specExponent = 200;
	vec3 specularItem =  material_.specular * light_.specularColor /* max(dot(point2light, _normal_), 0.f)*/
			* pow(max(dot(bisector, _normal_), 0.f),  material_.shiness); 

	// vec3 texColor = vec3(texture(texturer0_, _verTexCoords_));
	// vec3 result = texColor * (ambientItem + diffuseItem + specularItem);
	vec3 result = (ambientItem + diffuseItem + specularItem);
	_fragColor_ = vec4(result, 1.f);
}