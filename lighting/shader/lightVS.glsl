#version 330 core
	
// in 表示该数据需要从外部输入
// vec3 为变量类型, position 为变量名称
// 在 glVertexAttribPointer 函数中配置该位置，将 location的ID 告知 GPU, GPU 解析数据后将存到 posiotion 中
layout (location = 0) in vec3 position_v;

uniform mat4 model_;
uniform mat4 view_;
uniform mat4 project_;

void main()
{
	gl_Position = project_ * view_ * model_ * vec4(position_v, 1.0f);// 转换为齐次坐标
}