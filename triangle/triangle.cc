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


//生成一个[0,1] 随机数
float RandFrom0to1()
{
	Sleep(100);//这里为了让下一个随机数种子有时间产生一定的变化

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

	// 主循环
	glfwSwapInterval(1); // 设置前后缓冲区交换间隔，单位为帧
	while (!glfwWindowShouldClose(pWindow))
	{
		glfwPollEvents();//轮询 - glfw 与 窗口通信
		//glClearColor(0.3f, 0.5f, 0.7f, 1.0f);//设置清除颜色缓冲区后要使用的颜色-纯色
		glClearColor(RandFrom0to1(), RandFrom0to1(), RandFrom0to1(), 1.0f);//设置清除颜色缓冲区后要使用的颜色-动态变化
		glClear(GL_COLOR_BUFFER_BIT); // 清除颜色缓冲区
		glfwSwapBuffers(pWindow);//交换前后缓冲区
	}

	//清理资源
	glfwDestroyWindow(pWindow);//销毁窗口
	glfwTerminate();//终止GLFW
	exit(EXIT_SUCCESS);//退出
}