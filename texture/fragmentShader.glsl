#version 330 core

in vec3 transVerColor;
in vec2 transVerTexCoords;

out vec4 fragColor;
uniform sampler2D texturer; // 采样器会使用所设置的纹理参数进行采样

void main()
{
	// 前面几节的fragColor 都是直接使用的顶点颜色, 这里使用纹理采样颜色
	fragColor = texture(texturer, transVerTexCoords) * vec4(transVerColor, 1.0); // 这里 transVerColor, 需要转换为 4 分量
}