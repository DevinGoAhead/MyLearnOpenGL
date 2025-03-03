#version 330 core

layout (triangles) in; // 从顶点着色器输入 Triangle 图元
layout (triangle_strip, max_vertices = 5) out; // 设定输出图元类型和最大顶点数

in VS_OUT{
	vec2 texCoords;
}gmIn[];

out vec2 _texCoords_; // 输出到片段着色器中
out vec3 _position_; // 输出到片段着色器中

uniform float time_; // 当前时间

// 面法线
vec3 GetNormal() {
	vec3 v1to0 = vec3(gl_in[0].gl_Position - gl_in[1].gl_Position);
	vec3 v1to2 = vec3(gl_in[2].gl_Position - gl_in[1].gl_Position);
	// 课程中采用了相反方向的法向量, 显然模型的点的顺序是顺时针的
	// 我在 wxy::model 中导入模型阶段强制处理模型顶点顺序为 CCW, 故采用了如下的法向量方向
	return normalize(cross(v1to2, v1to0)); 
}

vec3 normal = GetNormal();

vec4 Explode(vec4 position) {
	vec3 dir = normal * ((sin(time_) + 1) / 2) * 1.5f; // (sin(time_) + 1) / 2 一个 (0, 1)的数, 1.5f 控制移动的程度(倍数)
	return position + vec4(dir, 0.f);
}

void main() {
	vec3 normal = GetNormal();
	gl_Position = Explode(gl_in[0].gl_Position);
	_texCoords_ = gmIn[0].texCoords;
	_position_ = vec3(gl_Position); // 裁剪空间
	EmitVertex();

	gl_Position = Explode(gl_in[1].gl_Position);
	_texCoords_ = gmIn[1].texCoords;
	_position_ = vec3(gl_Position); // 裁剪空间
	EmitVertex();

	gl_Position = Explode(gl_in[2].gl_Position);
	_texCoords_ = gmIn[2].texCoords;
	_position_ = vec3(gl_Position); // 裁剪空间
	EmitVertex();

	EndPrimitive();
}