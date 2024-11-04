/* GLFW 是配合 OpenGL 使用的轻量级工具程序库，缩写自 Graphics Library Framework（图形库框架）
 * ** 主要功能是创建并管理窗口和 OpenGL 上下文，同时还提供了处理手柄、键盘、鼠标输入的功能

 * GLAD 可以自动获取函数的地址
 * ** 自动生成加载器：你可以通过 GLAD 的网站生成一个适合你项目需求的加载器
 * ** 多语言支持：GLAD 可以扩展到其他编程语言，支持多种 OpenGL 规范
 * ** 独立于窗口库：GLAD 的加载器独立于窗口库，这意味着可以根据需要选择不同的窗口库
 */

#include "glad\glad.h"
#include "glfw\glfw3.h"

#include "classShaderProgram\shaderProgram.hpp"

#include <numbers>
#include <iostream>
#include <vector>
#include <utility>

#include <cstdio>
#include <ctime>
#include <cmath>

#include <windows.h>

// 空间中一点
struct Vertex
{
	GLfloat x;
	GLfloat y;
	GLfloat z;
};

// 三角形的三个点在 顶点vector 中的下标编号
struct Triangle
{
	GLuint aPtNum;
	GLuint bPtNum;
	GLuint cPtNum;
};

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
#ifdef DEBUG
	std::cout << "Current path: " << std::filesystem::current_path() << std::endl;
#endif
	glfwSetErrorCallback(error_callback); // 注册错误处理回调函数

	if (!glfwInit()) // 初始化GLFW, 并判断是否初始化成功
	{
		exit(EXIT_FAILURE);
	}

	// openGL 配置
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow *pWindow = glfwCreateWindow(800, 600, "first sphere", NULL, NULL);

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
	float aspectRatio = static_cast<float>(wndWidth) / static_cast<float>(wndHeight);
	if( aspectRatio > 1.0)//宽度 > 高度
		glViewport((wndWidth - wndHeight) / 2, 0, wndHeight, wndHeight);
	else//宽度 <= 高度
		glViewport((wndHeight - wndWidth) / 2, 0, wndWidth, wndWidth);

	// 注册视口尺寸自动调整回调函数
	glfwSetFramebufferSizeCallback(pWindow, framebuffer_size_callback);

	// 注册键盘动作处理回调函数(可选)
	glfwSetKeyCallback(pWindow, key_callback);

	// 创建着色器程序
	ShaderProgram myShaderProgram("../classShaderProgram/vertextShader.txt", "../classShaderProgram/fragmentShader.txt");


	/*这里为了避免两次遍历,将顶点数据的创建 和 顶点index 的创建放到了一个循环中*/
	
	
	// 将球划分出 LatitudeBands 个纬度(其实得到了 LatitudeBands + 1 条纬度线)
	// 将球划分出 LongitudeBands 个经度(其实得到了 LongitudeBands + 1 条纬度线,且第一条和最后一条是重合的)
	// 顶点总数其实是 (LatitudeBands +1) * (LongitudeBands + 1)个
	const int LatitudeBands = 40;  
	const int LongitudeBands = 80; 
	
	const GLfloat radius = 1.0f;	   // 球体半径
	
	std::vector<Vertex> vertices;
	Vertex pVertex;				   // 球上一点 P
	std::vector<Triangle> indices;
	Triangle triangle; //由几点扩展出的三角形

	// 每 ++ 一次, 纬度线递增一次, 自上之下遍历
	for (int latNum = 0; latNum <= LatitudeBands; ++latNum) 
	{
		// OP 与 y 轴的夹角, 范围为 [0, π]
		GLfloat phi = (static_cast<GLfloat>(latNum) / static_cast<GLfloat>(LatitudeBands)) * std::numbers::pi;

		for (int longiNum = 0; longiNum <= LongitudeBands; ++longiNum) // 每 ++ 一次, 经度线递增一次, 顺时针遍历
		{
			/* 创建顶点数据 */
			// OP 在 xOz 面投影线 ON 与 x 轴的夹角,范围为 [0, 2π]
			GLfloat theta = (static_cast<GLfloat>(longiNum) / static_cast<GLfloat>(LongitudeBands)) * 2 * std::numbers::pi;
			GLfloat lenON = radius * sin(phi); // ON 的长度

			pVertex.x = lenON * cos(theta);
			pVertex.y = radius * cos(phi);
			pVertex.z = lenON * sin(theta);
			vertices.push_back(pVertex);

			/* 创建顶点索引数据
	 		 * 每个基准点扩充出 2 个三角形, 忽略最后一行, 最后一列(行列是指如果能将球展开的情况下)
	 		 */
			if(latNum < LatitudeBands && longiNum < LongitudeBands)
			{
				//第一个三角形, 逆时针定义
				triangle.aPtNum = latNum * (LongitudeBands + 1) + longiNum;
				triangle.bPtNum = (latNum + 1) * (LongitudeBands + 1) + longiNum;
				triangle.cPtNum = (latNum + 1) * (LongitudeBands + 1) + (longiNum + 1);
				indices.push_back(triangle);

				//第二个三角形与第一个三角形共线 ac, 因此仅计算另一个点即可
				triangle.bPtNum = triangle.aPtNum + 1;
				std::swap(triangle.bPtNum, triangle.cPtNum);//确保逆时针定义三角形
				indices.push_back(triangle);
			}
		}
	}


	// /* 创建顶点索引数据
	//  * 每个基准点扩充出 2 个三角形, 忽略最后一行, 最后一列(行列是指如果能将球展开的情况下)
	//  */
	// std::vector<Triangle> indices;
	// Triangle triangle;
	// for (GLuint latNum = 0; latNum < LatitudeBands; ++latNum) 
	// {
	// 	for (GLuint longiNum = 0; longiNum < LongitudeBands; ++longiNum) // 每 ++ 一次, 经度线递增一次, 顺时针遍历
	// 	{
	// 		//第一个三角形, 逆时针定义
	// 		triangle.aPtNum = latNum * (LongitudeBands + 1) + longiNum;
	// 		triangle.bPtNum = (latNum + 1) * (LongitudeBands + 1) + longiNum;
	// 		triangle.cPtNum = (latNum + 1) * (LongitudeBands + 1) + (longiNum + 1);
	// 		indices.push_back(triangle);

	// 		//第二个三角形与第一个三角形共线 ac, 因此仅计算另一个点即可
	// 		triangle.bPtNum = triangle.aPtNum + 1;
	// 		std::swap(triangle.bPtNum, triangle.cPtNum);//确保逆时针定义三角形
	// 		indices.push_back(triangle);
	// 	}
	// }

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
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(Triangle), indices.data(), GL_STATIC_DRAW);

	// 告知 GPU 如何解析顶点数据
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(0);
	//std::cout << (void*)offsetof(Vertex, x) <<std::endl;
	glBindVertexArray(0); // 将vertexArray 从 OpenGL 当前上下文解绑

	// 使 透明值 生效
	glEnable(GL_BLEND);								   // 启用颜色混合操作功能 和 面剔除
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // 设置颜色混合模式
	
	glEnable(GL_CULL_FACE);// 启用 面剔除
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);//设置线框模式, 正向面和背向面都生效
	glCullFace(GL_BACK);//剔除背向面

	/*主循环*/
	glfwSwapInterval(1); // 设置前后缓冲区交换间隔，单位为帧
	while (!glfwWindowShouldClose(pWindow))
	{
		glfwPollEvents();					  // 轮询 - glfw 与 窗口通信
		glClearColor(0.3f, 0.5f, 0.7f, 1.0f); // 设置清除颜色缓冲区后要使用的颜色-纯色
		//glClearColor(RandFrom0to1(), RandFrom0to1(), RandFrom0to1(), 1.0f); // 设置清除颜色缓冲区后要使用的颜色-动态变化
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 清除颜色缓冲区 和 深度缓冲区

		// 设定 uniform 变量 fragColor 的值
		//myShaderProgram.SetUniform("fragColor", RandFrom0to1(), RandFrom0to1(), RandFrom0to1(), RandFrom0to1());
		myShaderProgram.SetUniform("fragColor", 0.9f, 0.7f, 0.5f, 1.0f);
		myShaderProgram.Use();			// 激活着色程序
		glBindVertexArray(vertexArray); // 第二次绑定同一个 VAO 时，OpenGL 会使用这个 VAO 中记录的所有配置信息来进行绘制操作
		glDrawElements(GL_TRIANGLES, 3 * indices.size(), GL_UNSIGNED_INT, (void *)0);

		glfwSwapBuffers(pWindow); // 交换前后缓冲区
	}
	// 清理资源
	glDeleteBuffers(1, &vertexBuffer);
	glDeleteBuffers(1, &elementBuffer);
	glBindVertexArray(0);
	glDeleteVertexArrays(1, &vertexArray);

	glfwDestroyWindow(pWindow); // 销毁窗口
	glfwTerminate();			// 终止GLFW
	exit(EXIT_SUCCESS);			// 退出
}