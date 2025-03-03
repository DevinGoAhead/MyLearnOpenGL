#version 330 core

layout (location = 0) in vec3 vertex_v;
layout (location = 1) in vec3 normal_v;
layout (location = 2) in vec2 texCoords_v;

// 会将所有点的texCoords 打包在一个 VS_OUT 类型的数组中, 输出到 几何着色器中
// 本例 texCoords 在几何着色器中也没有做任何处理, 这里仅为尝试使用接口快功能
out VS_OUT{
	vec2 texCoords;
}vsOut; 

out vec3 _normal_; // 直接传递到片段着色器
out mat4 _inverViewProj_; // 投影视图变换的逆矩阵, 直接传递到片段着色器

uniform mat4 model_;
uniform mat4 view_;
uniform mat4 projection_;

void main() {
	gl_Position = projection_ * view_ * model_ * vec4(vertex_v, 1.0f);// 转换为齐次坐标
	vsOut.texCoords = texCoords_v;
	_normal_ = mat3(transpose(inverse(model_))) * normal_v;
	_inverViewProj_ = inverse(projection_ * view_);
}