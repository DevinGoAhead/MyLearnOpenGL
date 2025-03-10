#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out VOUT {
	vec3 posWdSpace;
	vec3 normal;
	vec2 texCoord;
	// "光" 坐标系下的裁减空间, 即 Proj 变换后, 未裁减, 未透视除法
	// 这里必须要保留齐次坐标, 后面还要做透视除法.虽然本例正交投影没必要
	// posClipSpaceAtLgt 和 posWdSpace 本质是同一点在不同坐标空间的坐标
	vec4 posClipSpaceAtLgt;
} vOut;

// camera
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform mat4 uProjViewAtLgt; // "光" 坐标系下的投影变换 * 视图变换

void main() {
	vOut.posWdSpace = vec3(uModel * vec4(aPos, 1.f)); // 世界空间的顶点坐标
	vOut.normal = mat3(transpose(inverse(uModel))) * aNormal; // 世界空间中的法线
	vOut.texCoord = aTexCoord;
	vOut.posClipSpaceAtLgt = uProjViewAtLgt * vec4(vOut.posWdSpace, 1.f);
	gl_Position = uProjection * uView * vec4(vOut.posWdSpace, 1.f); // 观察坐标系下的裁减空间, 接下来由 图形管线 完成裁减和透视除法
}