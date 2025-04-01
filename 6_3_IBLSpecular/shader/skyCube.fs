#version 330 core

in vec3 vPosWd;
out vec4 fColor;

uniform samplerCube uTextureCube;
uniform float uMMapSampLevel;

void main(){
	//vec3 color = texture(uTextureCube, vPosWd).rgb;
	vec3 color = textureLod(uTextureCube, vPosWd, uMMapSampLevel).rgb;
	color = color / (color + vec3(1.0));
 	color = pow(color, vec3(1.0/2.2)); 

	//color = vec3(uMMapSampLevel / 4.f, 0.1, 0.1);
	fColor = vec4(color, 1.f);
}