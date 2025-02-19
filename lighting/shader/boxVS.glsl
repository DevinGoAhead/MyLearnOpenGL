#version 330 core
	
// in 表示该数据需要从外部输入
// vec3 为变量类型, position 为变量名称
// 在 glVertexAttribPointer 函数中配置该位置，将 location的ID 告知 GPU, GPU 解析数据后将存到 posiotion 中
layout (location = 0) in vec3 vertex_v;
layout (location = 1) in vec3 normal_v;
layout (location = 2) in vec2 verTexCoords_v;

uniform mat4 model_;
uniform mat4 view_;
uniform mat4 project_;

out vec3 _normal_;
out vec3 _vertex_;
out vec2 _verTexCoords_;


void main()
{
	gl_Position = project_ * view_ * model_ * vec4(vertex_v, 1.0f);// 转换为齐次坐标
	// 本例在世界空间中进行渲染, 故法线和顶点都传递到世界空间在 片段着色器中渲染
	// 法线矩阵要用逆转置, 才能确保和面(或其他向量)保持垂直
	// 强转为 3 * 3 矩阵再于 normal_v 相乘
	_normal_ =  mat3(transpose(inverse(model_))) * normal_v;
	_normal_ = normalize(_normal_); // 单位化
	_vertex_ = vec3(model_ * (vertex_v, 1.f));
	_verTexCoords_ = verTexCoords_v;
}