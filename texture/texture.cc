#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h" // 加载纹理图像
#include "glad\glad.h"
#include "glfw\glfw3.h"

#include "classShaderProgram\shaderProgram.hpp"

#include <iostream>
#include <cstdio>
#include <ctime>
#include <cmath>


#include <windows.h>

// 错误处理回调函数
void error_callback(int error, const char *description)
{
	fprintf(stderr, "ERROR: %s", description);
}

// 窗口尺寸自动调整回调函数
void framebuffer_size_callback(GLFWwindow *pWindow, int width, int height)
{
	glViewport(0, 0, width, height);
}

// 键盘按键处理回调函数
// 这里仅配置了Esc按键
void key_callback(GLFWwindow *pWindow, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(pWindow, GL_TRUE);
}

// 生成一个[0,1] 随机数
float RandFrom0to1()
{
	Sleep(100); // 这里为了让下一个随机数种子有时间产生一定的变化

	srand(time(NULL) | GetCurrentProcessId());
	double randNum = (int)glfwGetTime() | rand();
	return ((sin(randNum) / 2) + 0.5); // sin() / 2  [-0.5, +0.5]
}

int main()
{
	glfwSetErrorCallback(error_callback); // 注册错误处理回调函数

	if (!glfwInit()) // 初始化GLFW, 并判断是否初始化成功
	{
		exit(EXIT_FAILURE);
	}

	// openGL 配置
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	/*创建窗口*/
	//  800, 600 为逻辑像素
	// NULL-不创建全屏 NULL-不共享窗口上下文
	// 失败返回NULL
	GLFWwindow *pWindow = glfwCreateWindow(800, 600, "first triangle", NULL, NULL);

	if (!pWindow) // 判断是否创建成功
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMaximizeWindow(pWindow); // 最大化窗口

	glfwMakeContextCurrent(pWindow); // 将pWindow 设置为当前上下文

	// 初始化GLAD
	gladLoadGLLoader(GLADloadproc(glfwGetProcAddress));

	// 获取缓冲区的尺寸(物理像素)
	// 默认的帧缓冲区是屏幕帧缓冲区
	// 获取的区域并非整个屏幕，而是窗口的内容区域，不包括标题栏和边框
	int wndWidth, wndHeight;
	glfwGetFramebufferSize(pWindow, &wndWidth, &wndHeight);

	// 设定视口尺寸
	glViewport(0, 0, wndWidth, wndHeight);

	// 注册视口尺寸自动调整回调函数
	glfwSetFramebufferSizeCallback(pWindow, framebuffer_size_callback);
	
	// 注册键盘动作处理回调函数(可选)
	glfwSetKeyCallback(pWindow, key_callback);

	//创建着色器程序
	ShaderProgram myShaderProgram("./vertextShader.glsl", "./fragmentShader.glsl");

	/*创建顶点数据*/
	//          2- - -0
	//          |     |
	//          3- - -1

	GLfloat vertices[] = {
		//顶点坐标				顶点颜色		顶点对应的纹理坐标
		0.5f, 0.5f, 0.0f,    1.f, 1.f, 0.f,     1.f,1.f, // 0, 右上角
		0.5f, -0.5f, 0.0f,    1.f, 0.f, 1.f,     1.f,0.f, // 1, 右下角
		-0.5f, 0.5f, 0.0f,    0.f, 1.f, 1.f,     0.f,1.f, // 2, 左上角
		-0.5f, -0.5f, 0.0f,    1.f, 1.f, 1.f,     0.f,0.f // 3, 左下角
	};

	/*创建顶点索引数据*/
	// 这里必要性不大
	GLuint indices[] = {
		0, 2, 3,
		0, 3, 1
};

	/*创建缓冲对象*/
	GLuint vertexBuffer, elementBuffer; // 分别用于存储顶点缓冲对象, 元素(索引)缓冲对象
	glGenBuffers(1, &vertexBuffer);		// 创建一个缓冲对象,(将其ID)存储在 vertexBuffer 中
	glGenBuffers(1, &elementBuffer);	// 创建一个缓冲对象,(将其ID)存储在 elementBuffer 中

	// 创建纹理对象
	GLuint texture;
	glGenTextures(1, &texture); // 创建一个纹理对象, 将其 ID 存储在 texture 中
	
	/*顶点属性配置*/
	// 创建VAO, 以记录配置的属性
	// 这里由于点的数据量较小,所以是否使用VAO影响不大
	// 当点数据量很大时,我们只需要记录一次,即可重复使用,避免重复操作
	// 类似剪贴板,下次使用时,直接粘贴即可
	GLuint vertexArray;					// 用于存储顶点数组对象
	glGenVertexArrays(1, &vertexArray); // 创建一个顶点数组,(将其ID)存储在 vertexArray 中

	// 绑定 vertexArray 到 OpenGL 当前上下文, 在解绑之前, 所有对顶点数据的操作都会应用到这个VAO(配置信息会被记录)
	glBindVertexArray(vertexArray);

	// 在创建缓冲区对象时, 仅仅是在变量名称上区分了缓冲对象,其实它们本质没有区别,都是缓冲对象,下面实际对它们进行区分
	// 将 vertexBuffer 绑定在 GL_ARRAY_BUFFER 上,后续对 GL_ARRAY_BUFFER 的操作, 会影响 vertexBuffer
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	// 将 elementBuffer 绑定在 GL_ELEMENT_ARRAY_BUFFER 上,后续对 GL_ELEMENT_ARRAY_BUFFER 的操作, 会影响 vertexBuffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);
	
	// 将纹理图像绑定在 GL_TEXTURE_2D 上, 后续对 GL_TEXTURE_2D 的操作, 会影响 texture
	glBindTexture(GL_TEXTURE_2D, texture);

	// 设置纹理参数
	// 分别设置 ST 方向的环绕方式
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	
	// 设置纹理映射缩放算法
	// 当纹理被放大, 线性插值, 避免像素化
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
	// 当纹理被缩小, 使用 MipMap 算法, 且采用三线性插值的算法
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); 

	// 加载纹理对象
	int texWidth, texHeight, nRChannels;
	unsigned char* pImageData = stbi_load("./container.jpg", &texWidth, &texHeight, &nRChannels, 0);
	if(!pImageData) 
	{
		std::cerr << "failed to load Image" << '\n';
		exit(1);
	}
	
	// 生成纹理
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texWidth, texHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, pImageData);
	glGenerateMipmap(GL_TEXTURE_2D);// 自动生成多级渐远纹理
	stbi_image_free(pImageData);// 释放图片资源

	// 在 GPU 中开辟指定类型的缓冲区, 用于存放 vertices indeies
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// 告知 GPU 如何解析顶点数据
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	glBindVertexArray(0); // 将vertexArray 从 OpenGL 当前上下文解绑

	//使 透明值 生效
	glEnable(GL_BLEND);//启用颜色混合操作功能
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);//设置颜色混合模式

	/*主循环*/
	glfwSwapInterval(1); // 设置前后缓冲区交换间隔，单位为帧
	while (!glfwWindowShouldClose(pWindow))
	{
		glfwPollEvents(); // 轮询 - glfw 与 窗口通信
		glClearColor(0.3f, 0.5f, 0.7f, 1.0f);//设置清除颜色缓冲区后要使用的颜色-纯色
		//glClearColor(RandFrom0to1(), RandFrom0to1(), RandFrom0to1(), 1.0f); // 设置清除颜色缓冲区后要使用的颜色-动态变化
		glClear(GL_COLOR_BUFFER_BIT);										// 清除颜色缓冲区
		//glBindTexture(GL_TEXTURE_2D, texture);
		//myShaderProgram.SetUniform("fragColor", RandFrom0to1(), RandFrom0to1(), RandFrom0to1(), RandFrom0to1());
		myShaderProgram.Use();	// 激活着色程序
		glBindVertexArray(vertexArray); // 第二次绑定同一个 VAO 时，OpenGL 会使用这个 VAO 中记录的所有配置信息来进行绘制操作
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void *)0);

		glfwSwapBuffers(pWindow); // 交换前后缓冲区
	}
	// 清理资源
	glDeleteBuffers(1,&vertexBuffer);
	glDeleteBuffers(1,&elementBuffer);
	glBindVertexArray(0);
	glDeleteVertexArrays(1, &vertexArray);

	glfwDestroyWindow(pWindow); // 销毁窗口
	glfwTerminate();			// 终止GLFW
	exit(EXIT_SUCCESS);			// 退出
}