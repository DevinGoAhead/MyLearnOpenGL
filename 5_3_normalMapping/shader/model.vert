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

//uniform mat3 uTBN;
uniform vec3 uLightPos;
uniform vec3 uCameraPos;
uniform vec3 uT;
uniform vec3 uB;
uniform vec3 uN;

void main() {
	gl_Position = uProjection * uView * uModel * vec4(aPos, 1.f);

	/*** 这一版和示例代码相同, 但是我感觉我那种方式更容易理解 ***/
	mat3 transInvModel = transpose(inverse(mat3(uModel)));
	// 将 T B N 从基于模型坐标系转换到基于世界坐标系, 并且保持正交性
	vec3 T = normalize(transInvModel * uT);
	vec3 B = normalize(transInvModel * uB);
	vec3 N = normalize(transInvModel * uN);
	
	mat3 TBN = transpose(mat3(T,B, N));// 正交矩阵转置 = 逆
	
	// 将顶点, 光照位置 转换到切线空间
	// 先到模型空间, 再到切线空间
	vOut.posTan = TBN * vec3(uModel * vec4(aPos, 1.f)); // aPos -> 世界 -> 切线
	vOut.lightPosTan = TBN * uLightPos; // 世界 -> 切线
	vOut.cameraPosTan = TBN * uCameraPos; // 世界 -> 切线
	vOut.texCoord = aTexCoord;
}