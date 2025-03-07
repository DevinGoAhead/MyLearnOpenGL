#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 0) in vec3 aNormal;
layout (location = 0) in vec2 aTexcoord;

out vec3 vPos;
out vec3 vNormal;
out vec2 vTexCoord;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

void main() {
	gl_Position = uProjection * uView * uModel * vec4(aPos, 1.f);
	vPos = vec3(uModel * vec4(aPos, 1.f));
	vNormal = mat3(transpose(inverse(uModel))) * aNormal;
	vTexCoord = aTexcoord;
}
