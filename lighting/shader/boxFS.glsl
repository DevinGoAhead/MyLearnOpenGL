#version 330 core

struct Material
{
	sampler2D diffuseTexer;
	sampler2D specularTexer;
	float shiness;
};

//聚光
struct Light
{
	vec3 ambientColor;
	vec3 diffuseColor;
	vec3 specularColor;
	vec3 pos;
	float constant; // "光强" 衰减常数项
	float linear; // "光强" 衰减一次项(线性衰减)
	float quadratic; // "光强" 衰减二次项
	vec3 direc;
	float phi; // cos(inCutOffAngle)
	float gamma; // cos(outCutOffAngle)
};

uniform vec3 cameraPos_;
uniform struct Material material_;
uniform struct Light light_;

in vec3 _normal_;
in vec3 _vertex_;
out vec4 _fragColor_;
in vec2 _verTexCoords_;

void main()
{
	vec3 normal = normalize(_normal_);
	float distance = length(light_.pos - _vertex_);
	float attenuation = 1.f / ( light_.constant + light_.linear * distance +  light_.quadratic *  distance * distance);
	attenuation = 1.f;
	
	vec3 point2light = normalize(light_.pos - _vertex_); //点指向光源, 单位化
	float theta = dot(-point2light, light_.direc); // dot(light2point, spotDirec)
	float K = clamp((theta - light_.gamma) / (light_.phi - light_.gamma), 0.f, 1.0f);
	// 环境光使用和漫反射相同的材质颜色
	// 本例设定 K 不影响环境光. 避免全黑
	vec3 ambientItem = vec3(texture(material_.diffuseTexer, _verTexCoords_ )) * light_.ambientColor * attenuation; 
	
	//_normal_ = normalize(_normal_); // 单位化, varing 变量, 即从外面传递过来的变量, 不能修改, 所以智能在顶点着色器中单位化
	vec3 diffuseItem = vec3(texture(material_.diffuseTexer, _verTexCoords_ )) * light_.diffuseColor * attenuation * K * max(dot(point2light, normal), 0.f);

	vec3 point2eye = normalize(cameraPos_ - _vertex_); //点指向观察位置, 单位化
	vec3 bisector = normalize(point2eye + point2light); //半程向量
	
	vec3 specularItem = vec3(texture(material_.specularTexer, _verTexCoords_ )) * light_.specularColor * attenuation * K /** max(dot(point2light, normal), 0.f)*/
			* pow(max(dot(bisector, normal), 0.f),  material_.shiness); 
	vec3 result = (ambientItem + diffuseItem + specularItem);
	_fragColor_ = vec4(result, 1.f);
}