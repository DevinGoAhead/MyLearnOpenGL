#version 330 core

in vec3 vPosWd;
out vec4 fColor;

uniform samplerCube uTextureCube;

void main(){
	vec3 color = texture(uTextureCube, vPosWd).rgb;
	color = color / (color + vec3(1.0));
 	color = pow(color, vec3(1.0/2.2)); 

	//color = vec3(0.4, 0.6, 0.8);
	fColor = vec4(color, 1.f);
}