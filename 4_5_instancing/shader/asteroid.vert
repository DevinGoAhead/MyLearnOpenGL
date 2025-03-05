#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in mat4 aModel;

out vec2 vTexCoord;
out vec3 vPos;
out vec3 vNormal;

uniform mat4 uView;
uniform mat4 uProjection;

void main() {
	gl_Position = uProjection * uView * aModel * vec4(aPos, 1.f);
	vPos = vec3(aModel * vec4(aPos, 1.f));
	vNormal = mat3(transpose(inverse(aModel))) * aNormal;
	vTexCoord = aTexCoord;
}