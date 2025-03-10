#version 330 core

in vec4 gPos;//世界坐标系

uniform float uFar;
uniform vec3 uLightPos;

void main() {
	float lightPtLen = length(uLightPos - gPos.xyz); // 光和point 的实际距离
	lightPtLen /= uFar; // 变换到[0, 1], 线性
	gl_FragDepth = lightPtLen; // 设置片段的深度值, 图形管线会完成深度测试
}