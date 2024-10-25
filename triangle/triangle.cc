/* GLFW 是配合 OpenGL 使用的轻量级工具程序库，缩写自 Graphics Library Framework（图形库框架）
 * ** 主要功能是创建并管理窗口和 OpenGL 上下文，同时还提供了处理手柄、键盘、鼠标输入的功能

 * GLAD 可以自动获取函数的地址
 * ** 自动生成加载器：你可以通过 GLAD 的网站生成一个适合你项目需求的加载器
 * ** 多语言支持：GLAD 可以扩展到其他编程语言，支持多种 OpenGL 规范
 * ** 独立于窗口库：GLAD 的加载器独立于窗口库，这意味着可以根据需要选择不同的窗口库
 */

#include "glad\glad.h"
#include "glfw\glfw3.h"

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

// 顶点着色器源码
const char *vertexShaderCode = R"(
	#version 330 core
	
	// in 表示该数据需要从外部输入
	// vec3 为变量类型, position 为变量名称
	// 在 glVertexAttribPointer 函数中配置该位置，将 location的ID 告知GPU, GPU 解析数据后将存到 posiotion 中
	layout (location = 0) in vec3 position;
	out vec4 fragColor;//out 表示输出, 该变量将输出至片段着色器

	void main()
	{
		gl_Position = vec4(position, 1.0f);//齐次坐标, openGL内建变量, 表示点在裁剪空间的位置,本例给出NDC内坐标, 避免复杂转换
		fragColor = vec4(0.7f, 0.5f, 0.3f, 0.4f); //颜色处理
	}
)";

// 片段着色器源码
const char *fragmentShaderCode = R"(
	#version 330 core

	// fragColor 需要与顶点着色器中的输出变量 fragColor 的名称,类型均相同,数据才能传递过来
	in vec4 fragColor;
	out vec4 color;//输出变量,输出到下一个阶段

	void main()
	{
		color = fragColor;//颜色处理,简单示意
	}
)";

// 编译着色器, 并获取编译日志
void isCompileSuccess(GLuint shader)
{
	
	GLint compileStatus = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus); // 获取着色器编译状态
	// 如果编译失败,报错退出
	if (!compileStatus)
	{
		std::cout << "ERROR::SHADER::COMPILE_FAILED\n"
				  << std::endl;
		//日志
		GLchar log[1024];
		glGetShaderInfoLog(shader, 1023, NULL, log); // 字符串存储编译日志

		// 将日志存储到本地
		time_t now;
		time(&now);
		struct tm *local = localtime(&now);
		char fileName[64];
		// sprintf(fileName, "compile_log_%s",ctime(&now));//文件格式为compile_log_ + 时间
		strftime(fileName, sizeof(fileName), "compile_log_%Y-%m-%d.txt", local);

		FILE *logFile = fopen(fileName, "a");
		fprintf(logFile, "%s\n", log);
		fclose(logFile);
		exit(EXIT_FAILURE);
	}
}

// 链接程序对象, 并获取链接日志
void isLinkSuccess(GLuint shaderProgram)
{
	// 如果编译失败,报错,打印日志, 退出
	GLint linkStatus = 0;
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linkStatus); // 获取程序链接状态
	if (!linkStatus)
	{
  		std::cout << "ERROR::SHADER::LINK_FAILED\n"
				  << std::endl;
		//日志
		GLchar log[1024];
		glGetProgramInfoLog(shaderProgram, 1023, NULL, log); // 字符串存储链接日志

		// 将日志存储到本地
		time_t now;
		time(&now);
		struct tm *local = localtime(&now);
		char fileName[64];
		// sprintf(fileName, "link_log_%s",ctime(&now));//文件格式为compile_log_ + 时间
		strftime(fileName, sizeof(fileName), "link_log_%Y-%m-%d.txt", local);

		FILE *logFile = fopen(fileName, "a");
		fprintf(logFile, "%s\n", log);

		fclose(logFile);
		exit(EXIT_FAILURE);
	}
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
	// //v1 这样的方式不显示标题栏、边框
	// GLFWmonitor *priMonitor = glfwGetPrimaryMonitor();		   // 返回指向主显示器的指针
	// const GLFWvidmode *vidMode = glfwGetVideoMode(priMonitor); // 返回 priMonitor 的视频模式

	// GLFWwindow *pWindow = glfwCreateWindow(vidMode->width, vidMode->height,
	// 					"Triangle-Single Color", priMonitor, NULL);

	// v2
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

	/*创建并编译着色器*/
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);		// 创建顶点着色器
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER); // 创建片段着色器

	// 使用目标着色器源码覆盖着色器中旧的源码(因为现在着色器是新建的,所以这里也可以说是填充源码)
	glShaderSource(vertexShader, 1, &vertexShaderCode, NULL);
	glShaderSource(fragmentShader, 1, &fragmentShaderCode, NULL);

	// 编译着色器并检测是否编译成功
	glCompileShader(vertexShader);
	isCompileSuccess(vertexShader);
	
	glCompileShader(fragmentShader);
	isCompileSuccess(fragmentShader);

	/*创建程序对象*/
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);			   // 链接
	isLinkSuccess(shaderProgram);

	/*创建顶点数据*/
	//          1- 0-3
	//          | /\ |
	//         2|/  \|4
	//          |\  /|
	//          | \/ |
	//          6- 5-7
	GLfloat vertices[] = {
		0.0f, 0.3f, 0.0f,  // 0
		-0.2f, 0.3f, 0.0f, // 1
		-0.2f, 0.0f, 0.0f, // 2

		// 0.0f, 0.3f, 0.0f, // 0
		0.2f, 0.3f, 0.0f, // 3
		0.2f, 0.0f, 0.0f, // 4

		0.0f, -0.3f, 0.0f,	// 5
		-0.2f, -0.3f, 0.0f, // 6
		//-0.2f, 0.0f,  0.0f, // 2

		// 0.0f, -0.3f, 0.0f, // 5
		0.2f, -0.3f, 0.0f, // 7
						   // 0.2f,  0.0f, 0.0f, // 4
	};

	/*创建顶点索引数据*/
	GLuint indices[] = {
		0, 1, 2,
		0, 3, 4,
		5, 6, 2,
		5, 7, 4};

	/*创建缓冲对象*/
	GLuint vertexBuffer, elementBuffer; // 分别用于存储顶点缓冲对象, 元素(索引)缓冲对象
	glGenBuffers(1, &vertexBuffer);		// 创建一个缓冲对象,(将其ID)存储在 vertexBuffer 中
	glGenBuffers(1, &elementBuffer);	// 创建一个缓冲对象,(将其ID)存储在 elementBuffer 中

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

	// 在 GPU 中开辟指定类型的缓冲区, 用于存放 vertices indeies
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// 告知 GPU 如何解析顶点数据
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0); // 将vertexArray 从 OpenGL 当前上下文解绑

	/*主循环*/
	glfwSwapInterval(1); // 设置前后缓冲区交换间隔，单位为帧
	while (!glfwWindowShouldClose(pWindow))
	{
		glfwPollEvents(); // 轮询 - glfw 与 窗口通信
		glClearColor(0.3f, 0.5f, 0.7f, 1.0f);//设置清除颜色缓冲区后要使用的颜色-纯色
		//glClearColor(RandFrom0to1(), RandFrom0to1(), RandFrom0to1(), 1.0f); // 设置清除颜色缓冲区后要使用的颜色-动态变化
		glClear(GL_COLOR_BUFFER_BIT);										// 清除颜色缓冲区

		glUseProgram(shaderProgram);	// 激活着色程序
		glBindVertexArray(vertexArray); // 第二次绑定同一个 VAO 时，OpenGL 会使用这个 VAO 中记录的所有配置信息来进行绘制操作
		glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, (void *)0);

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