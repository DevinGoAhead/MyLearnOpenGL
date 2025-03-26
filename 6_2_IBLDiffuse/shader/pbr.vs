#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out VOUT {
	vec3 pos;
	vec3 normal;
	vec2 texCoords;
} vOut;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

void main() {
	vOut.pos = (uModel * vec4(aPos, 1.f)).xyz;
	vOut.normal = mat3(transpose(inverse(uModel))) * aNormal;
	vOut.texCoords = aTexCoords;
	gl_Position = uProjection *  uView * vec4(vOut.pos, 1.f);
}