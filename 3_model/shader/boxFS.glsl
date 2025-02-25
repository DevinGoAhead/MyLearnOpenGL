#version 330 core
out vec4 _FragColor_;

in vec2 _TexCoords_;

uniform sampler2D textureDiffuse0_;
uniform sampler2D textureDiffuse1_;

void main()
{    
    _FragColor_= texture(textureDiffuse0_, _TexCoords_);
	//FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}