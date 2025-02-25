#version 330 core

struct Material
{
	sampler2D diffuseTexer;
	sampler2D specularTexer;
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

//电光
struct PointLight
{
	vec3 ambientColor;
	vec3 diffuseColor;
	vec3 specularColor;
	vec3 pos;
};

//聚光
struct SpotLight
{
	vec3 ambientColor;
	vec3 diffuseColor;
	vec3 specularColor;
	vec3 pos;
	vec3 direc;
	float phi; // cos(inCutOffAngle)
	float gamma; // cos(outCutOffAngle)
};

struct Attenuation
{
	float constant; // "光强" 衰减常数项
	float linear; // "光强" 衰减一次项(线性衰减)
	float quadratic; // "光强" 衰减二次项
};

#define NUM_POINT_LIGHT 4
uniform vec3 cameraPos_;
uniform struct Material material_;
uniform struct DirecLight direcLight_;
uniform struct PointLight pointLight_[NUM_POINT_LIGHT];
uniform struct SpotLight spotLight_;
uniform struct Attenuation attenuation_;


in vec3 _normal_;
in vec3 _vertex_;
out vec4 _fragColor_;
in vec2 _verTexCoords_;

vec3 normal = normalize(_normal_);

// 上面的变量类似全局变量, 可以直接在函数中使用

// 计算不含K 和 attenuation 的AmbientItem
vec3 InitAmbientItem(vec3 ambientColor){ 
	return vec3(texture(material_.diffuseTexer, _verTexCoords_ )) * ambientColor;
}

// 计算不含K 和 attenuation 的AmbientItem, 不含 cosine law
vec3 InitDiffuseItem(vec3 diffuseColor){
	return vec3(texture(material_.diffuseTexer, _verTexCoords_ )) * diffuseColor;
}

// 计算不含K 和 attenuation 的AmbientItem, 不含夹角...
vec3 InitSpecularItem(vec3 specularColor){
	return vec3(texture(material_.specularTexer, _verTexCoords_ )) * specularColor;
} 

vec3 CalPointLight(struct PointLight pointLight)
{
	vec3 point2light = normalize(pointLight.pos - _vertex_); //点指向光源, 单位化
	
	float distance = length(pointLight.pos - _vertex_);
	float attenuation = 1.f / ( attenuation_.constant + attenuation_.linear * distance +  attenuation_.quadratic *  distance * distance);
	
	vec3 ambientItem = InitAmbientItem(pointLight.ambientColor) * attenuation;// 本例设定 K 不影响环境光. 避免全黑
	vec3 diffuseItem = InitDiffuseItem(pointLight.diffuseColor) * max(dot(point2light, normal), 0.f) * attenuation;

	vec3 point2eye = normalize(cameraPos_ - _vertex_); //点指向观察位置, 单位化
	vec3 bisector = normalize(point2eye + point2light); //半程向量
	vec3 specularItem = InitSpecularItem(pointLight.specularColor) * attenuation * pow(max(dot(bisector, normal), 0.f),  material_.shiness); 
	
	return ambientItem + diffuseItem + specularItem;
}

vec3 CalDirecLight()
{
	vec3 point2light = normalize(-direcLight_.direc); //平行光, 光源方向与物体(的点)无光, 不需要与点做运算, 直接用光本身的方向
	vec3 ambientItem = InitAmbientItem(direcLight_.ambientColor);
	vec3 diffuseItem = InitDiffuseItem(direcLight_.diffuseColor) * max(dot(point2light, normal), 0.f);

	vec3 point2eye = normalize(cameraPos_ - _vertex_); //点指向观察位置, 单位化
	vec3 bisector = normalize(point2eye + point2light); //半程向量
	vec3 specularItem = InitSpecularItem(direcLight_.specularColor) * pow(max(dot(bisector, normal), 0.f),  material_.shiness); 
	
	return ambientItem + diffuseItem + specularItem;
}
vec3 CalSpotLight()
{
	vec3 point2light = normalize(spotLight_.pos - _vertex_); //点指向光源, 单位化
	
	float theta = dot(-point2light, spotLight_.direc); // dot(light2point, spotDirec)
	float K = clamp((theta - spotLight_.gamma) / (spotLight_.phi - spotLight_.gamma), 0.f, 1.0f);
	
	float distance = length(spotLight_.pos - _vertex_);
	float attenuation = 1.f / ( attenuation_.constant + attenuation_.linear * distance +  attenuation_.quadratic *  distance * distance);
	
	vec3 ambientItem = InitAmbientItem(spotLight_.ambientColor) * attenuation;// 本例设定 K 不影响环境光. 避免全黑
	vec3 diffuseItem = InitDiffuseItem(spotLight_.diffuseColor) * max(dot(point2light, normal), 0.f) * attenuation * K;

	vec3 point2eye = normalize(cameraPos_ - _vertex_); //点指向观察位置, 单位化
	vec3 bisector = normalize(point2eye + point2light); //半程向量
	vec3 specularItem = InitSpecularItem(spotLight_.specularColor) * attenuation * K * pow(max(dot(bisector, normal), 0.f),  material_.shiness); 
	
	return ambientItem + diffuseItem + specularItem;
}

void main()
{
	vec3 resultPointLight;
	for(int i = 0; i < NUM_POINT_LIGHT; ++i){
		resultPointLight += CalPointLight(pointLight_[i]);
	}
	_fragColor_ = vec4(CalSpotLight() + CalDirecLight() + resultPointLight, 1.f);
}