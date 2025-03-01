#version 330 core

layout (location = 0) in vec3 vertex_v;

out vec3 _TexCoords_;

uniform mat4 model_;
uniform mat4 view_;
uniform mat4 projection_;

void main() {
	_TexCoords_ = vertex_v; // 天空盒本身就在世界坐标系

	vec4 position = projection_ * view_ * vec4(vertex_v, 1.0f);// 转换为齐次坐标
	// 令透视除法前的 z 分量手动设置为 w
	// 这样透视除法后在 NDC 中, 其 z 将 == 1
	// 即使经过 depth mapping 后, 仍然为1, 其将保持为 "最远"
	gl_Position = position.xyww; 
}