#version 330 core

in vec3 transVerColor;
in vec2 transVerTexCoords;

out vec4 fragColor;
uniform sampler2D texturer0; // 采样器会使用所设置的纹理参数进行采样
uniform sampler2D texturer1; // 采样器会使用所设置的纹理参数进行采样

void main()
{
	// 前面几节的fragColor 都是直接使用的顶点颜色, 这里使用纹理采样颜色
	// 这里分别采样两个纹理图像并混合, 0.2 表示后者所占比例
	fragColor = mix(texture(texturer0, transVerTexCoords), texture(texturer1, transVerTexCoords), 0.2);
}