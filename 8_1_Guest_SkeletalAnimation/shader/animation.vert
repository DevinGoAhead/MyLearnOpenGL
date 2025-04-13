#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

layout (location = 4) in ivec4 aBoneIDs;
layout (location = 5) in vec4 aWeights;

#define MAXINFLUENCEBONE 4
#define MAXBONES 100

out vec2 vTexCoord;
out vec4 vAveragePos;
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform mat4 uFinalTransforms[MAXBONES];

void main() {
	vec4 averagePos = vec4(0.f);
	for(int i = 0; i < MAXINFLUENCEBONE; ++i) {
		// learnOpenGL 对 aBoneIDs[i] >= MAXBONES 的处理是break, 原因在于 learnOpenGL 中的 aBoneIDs[i] 是单调递增
		// 而我进行了按weight 重排, 截断, 因而即使当前  aBoneIDs[i] >= MAXBONES, 后面却不一定 >
		if(aBoneIDs[i] == -1 || aBoneIDs[i] >= MAXBONES) continue;
		vec4 curLocalPos = uFinalTransforms[aBoneIDs[i]] * vec4(aPos, 1.f);
		averagePos += curLocalPos * aWeights[i];
	}

	gl_Position = uProjection * uView * uModel * averagePos;
	//gl_Position = uProjection * uView * uModel * vec4(aPos, 1.f);
	vTexCoord = aTexCoord;
}
