#version 330 core
	
// in 表示该数据需要从外部输入
// vec3 为变量类型, position 为变量名称
// 在 glVertexAttribPointer 函数中配置该位置，将 location的ID 告知 GPU, GPU 解析数据后将存到 posiotion 中
layout (location = 0) in vec2 vertex_v;
layout (location = 1) in vec2 verTexCoords_v;

out vec2 _verTexCoords_;

void main()
{
	gl_Position = vec4(vertex_v.x, vertex_v.y, 0.f, 1.0f);// 转换为齐次坐标
	_verTexCoords_ = verTexCoords_v;
}