#version 330 core
out vec4 _FragColor_;

in vec2 _TexCoords_;
struct Material {
	sampler2D textureDiffuse0;
	sampler2D textureSpecular0;
	sampler2D textureDiffuse1;
	sampler2D textureSpecular1;
};

uniform Material material_;

void main()
{    
    _FragColor_= texture(material_.textureDiffuse0, _TexCoords_);
	//_FragColor_=vec4(_TexCoords_, 0.f, 1.f) ; // 正常
	//FragColor = vec4(1.0, 0.0, 0.0, 1.0); // 可显示
}