#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out VOUT {
	vec3 posTan; //切线空间
	vec3 lightPosTan; // 切线空间
	vec3 cameraPosTan; // 切线空间
	vec2 texCoord;
} vOut;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

uniform mat3 uTBN;
uniform vec3 uLightPos;
uniform vec3 uCameraPos;

void main() {
	gl_Position = uProjection * uView * uModel * vec4(aPos, 1.f);

	mat3 invTBN = transpose(uTBN);
	mat3 invModel = inverse(mat3(uModel));
	
	// 将顶点, 光照位置 转换到切线空间
	vOut.posTan = uTBN * aPos; // 模型 -> 切线
	vOut.lightPosTan = uTBN * invModel * uLightPos; // 世界 -> 模型 -> 切线
	vOut.cameraPosTan = uTBN * invModel * uCameraPos;// 世界 -> 模型 -> 切线
	vOut.texCoord = aTexCoord;
}