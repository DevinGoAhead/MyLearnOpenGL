#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aTangent;

out VOUT {
	vec3 posTan; //切线空间
	vec3 lightPosTan; // 切线空间
	vec3 cameraPosTan; // 切线空间
	vec2 texCoord;
} vOut;

out vec3 vNormal;
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

//uniform mat3 uTBN;
uniform vec3 uLightPos;
uniform vec3 uCameraPos;

void main() {
	gl_Position = uProjection * uView * uModel * vec4(aPos, 1.f);

	mat3 transInvModel = transpose(inverse(mat3(uModel)));
	// 将 T B N 从基于模型坐标系转换到基于世界坐标系, 并且保持正交性
	vec3 T = aTangent - (aNormal * (dot(aNormal, aTangent))); // 正交化
	
	T = normalize(transInvModel * aTangent);
	vec3 N = normalize(transInvModel * aNormal);
	vec3 B = cross(N, T);
	mat3 TBN = transpose(mat3(T,B, N));// 正交矩阵转置 = 逆
	
	// 将顶点, 光照位置 转换到切线空间
	// 先到模型空间, 再到切线空间
	vOut.posTan = TBN * vec3(uModel * vec4(aPos, 1.f)); // aPos -> 世界 -> 切线
	vOut.lightPosTan = TBN * uLightPos; // 世界 -> 切线
	vOut.cameraPosTan = TBN * uCameraPos; // 世界 -> 切线
	vOut.texCoord = aTexCoord;

	// vOut.posTan = vec3(uModel * vec4(aPos, 1.f)); // aPos -> 世界 -> 切线
	// vOut.lightPosTan = uLightPos; // 世界 -> 切线
	// vOut.cameraPosTan = uCameraPos; // 世界 -> 切线
	// vOut.texCoord = aTexCoord;
	// vNormal = transInvModel * aNormal;
	
}