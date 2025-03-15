#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out VOUT {
	vec3 pos;
	vec3 normal;
	vec2 texCoord;
} vOut;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;


void main() {
	vOut.pos = vec3(uModel * vec4(aPos, 1.f));
	vOut.normal = mat3(transpose(inverse(uModel))) * aNormal;
	vOut.texCoord = aTexCoord;
	gl_Position = uProjection * uView * vec4(vOut.pos, 1.f);
}