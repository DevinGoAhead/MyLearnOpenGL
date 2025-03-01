#version 330 core

in vec3 _vertex_;
in vec3 _normal_;
out vec4 _fragColor_;

uniform samplerCube texerSkyBox_;
uniform vec3 cameraPos_;

void main() {
	vec3 vertex = _vertex_;
	vec3 normal = normalize(_normal_);
	
	vec3 camera2point = vertex - cameraPos_;
	vec3 reflectDirec = reflect(camera2point, normal);
	_fragColor_ = texture(texerSkyBox_, reflectDirec);
}