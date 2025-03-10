#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 18) out; // 每次分别向每个面输出一个三角形, 共 18 个点

out vec4 gPos; // 世界坐标系位置, 输出到frag shader
uniform mat4 uLightProjViewMats[6];// 这些都是以 光源为基准的变换矩阵

void main() {
	for(int iFace = 0; iFace < 6; ++iFace) {
		gl_Layer = iFace; // 指定接下来的图元绘制到cube 的哪一个面
		for(int iPos = 0; iPos < 3; ++iPos){
			gPos = gl_in[iPos].gl_Position;
			gl_Position = uLightProjViewMats[iFace] * gPos; // 图形管线会完成裁剪和透视除法
			EmitVertex();
		}
		EndPrimitive(); // 一个图元绘制完成
	}
}
