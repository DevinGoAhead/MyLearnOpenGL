#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h" // 加载纹理图像

#include "glad\glad.h"
#include "glfw\glfw3.h"

#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"
#include "glm\gtc\type_ptr.hpp"

#include "MyClass\shaderProgram.hpp"

#include <iostream>
#include <cstdio>
#include <ctime>
#include <cmath>

#include <windows.h>

// window
int wndWidth, wndHeight;

//eye
glm::vec3 eyePos(0.f, 0.f, -2.f);
glm::vec3 eyeFront(0.f, 0.f, -1.f); // 这里指的是 eye 看向的正方向, 即 eyePos - pixelCoord并单位化, 而不是实际看向物体的方向
glm::vec3 eyeUp(0.f, 1.f, 0.f);
double pitch = 0.f, yaw = -90.f; // pitch 与 y 轴夹角, yaw 与 x 轴夹角
float fov = 45.f;

// time
float curTime = 0.f, lastTime = 0.f, perFrameTime = 0.f;

//cursor
double xLast = wndWidth / 2, yLast = wndHeight / 2; // 初始值为视口中心

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
	float speed = 8 * perFrameTime;
	switch (key)
	{
		case GLFW_KEY_ESCAPE:
		{
			if(action == GLFW_PRESS)
				glfwSetWindowShouldClose(pWindow, GL_TRUE);
			break;
		}
		case GLFW_KEY_W : // 前
		{
			eyePos += (speed * eyeFront);
			break;
		}
		case GLFW_KEY_S : // 后
		{
			eyePos -= (speed * eyeFront);
			break;
		}
		case GLFW_KEY_A : // 左
		{
			eyePos -= speed * glm::normalize(glm::cross(eyeFront, eyeUp));
			break;
		}
		case GLFW_KEY_D : // 右
		{
			eyePos += speed * glm::normalize(glm::cross(eyeFront, eyeUp));
			break;
		}
		default:
		{

		}
	}
}

bool first = true;
void cursor_callback(GLFWwindow* window, double xPos, double yPos)
{
	// 计算偏移值
	// 当我们想向上时, 理应 yPos - yLast > 0, 但屏幕 y 坐标向下为正, 导致 yPos - yLast < 0, 故取反
	double dx = xPos - xLast, dy = yLast - yPos;
	if(first) {dx = 0, dy =0, first = false;}

	xLast = xPos, yLast = yPos;
	double sensitivity = 0.05f; // 这个值自己调整
	dx *= sensitivity, dy *= sensitivity;

	pitch += dy, yaw += dx;// 添加到俯仰角中

	// 限制角度不要转到异常角度
	if(pitch > 89.f) pitch = 89.f;
	if(pitch < -89.f) pitch = -89.f;
	
	// 计算新的eyeFront
	eyeFront.y = glm::sin(glm::radians(pitch));
	double rXZ = glm::cos(glm::radians(pitch));
	eyeFront.x = rXZ * glm::cos(glm::radians(yaw));
	eyeFront.z = rXZ * glm::sin(glm::radians(yaw));
	eyeFront = glm::normalize(eyeFront);
}

// 多数鼠标无法在 x 方向滚动, 故暂忽略 xoffset
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	// 通常滚轮向下是一个放大的操作, 而滚轮向下会导致一个 > 0 的 yoffset, 此时我们想
	fov += yoffset;
	if(fov > 50.f) fov = 50.f;
	if(fov < 0.1f) fov = 0.1f;
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
	glfwGetFramebufferSize(pWindow, &wndWidth, &wndHeight);
	
	glViewport(0, 0, wndWidth, wndHeight);// 设定视口尺寸
	glfwSetInputMode(pWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN); // 隐藏光标

	glfwSetFramebufferSizeCallback(pWindow, framebuffer_size_callback);// 注册视口尺寸自动调整回调函数
	
	glfwSetKeyCallback(pWindow, key_callback);// 注册键盘动作处理回调函数(可选)
	glfwSetCursorPosCallback(pWindow, cursor_callback);// 注册光标捕捉函数
	glfwSetScrollCallback(pWindow, scroll_callback);// 注册滚轮捕捉函数

	//创建着色器程序
	ShaderProgram myShaderProgram("./vertextShader.glsl", "./fragmentShader.glsl");

	/*创建顶点数据*/
	GLfloat vertices[] = {
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
		0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
		0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	   -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
	   -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

	   -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
	   -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
	   -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

	   -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	   -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	   -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	   -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	   -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	   -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

	   -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
		0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	   -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	   -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

	   -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	   -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
	   -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
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
	GLuint texture0, texture1;
	glGenTextures(1, &texture0); // 创建一个纹理对象, 将其 ID 存储在 texture0 中
	glGenTextures(1, &texture1); // 创建一个纹理对象, 将其 ID 存储在 texture1 中
	
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
	
	// 设置纹理参数
	auto TexParameteri = [](){
		// 设置纹理参数
		// 分别设置 ST 方向的环绕方式
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		// 设置纹理映射缩放算法
		// 当纹理被放大, 线性插值, 避免像素化
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
		// 当纹理被缩小, 使用 MipMap 算法, 且采用三线性插值的算法
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); 
	};

	// 加载并生成纹理图像
	auto GenerateTexImg = [](const char* pathName, GLint internalFormat){
		// 加载图像
		stbi_set_flip_vertically_on_load(true); // 加载图像时反转 y 轴
		int texWidth, texHeight, nRChannels;
		unsigned char* pImageData = stbi_load(pathName, &texWidth, &texHeight, &nRChannels, 0);
		if(!pImageData) 
		{
			std::cerr << "failed to load Image" << '\n';
			exit(1);
		}
		// 生成纹理
		// 生成纹理图像
		GLenum format;
		switch (nRChannels)
		{
			case 3:
			{
				format = GL_RGB;
				break;
			}
			case 4:
			{
				format = GL_RGBA;
				break;
			}
			default:
			{}
		}
		// 生成的纹理图像将保存在 glGenTextures 所生成的对象中
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, texWidth, texHeight, 0, format, GL_UNSIGNED_BYTE, pImageData);
		glGenerateMipmap(GL_TEXTURE_2D);// 自动生成多级渐远纹理
		stbi_image_free(pImageData);// 释放图片资源
	};

	// 处理texture0
	// 将纹理图像绑定在 GL_TEXTURE_2D 上, 后续对 GL_TEXTURE_2D 的操作, 会影响 texture
	glActiveTexture(GL_TEXTURE0); // 激活纹理单元0, 并绑定 texture0
	glBindTexture(GL_TEXTURE_2D, texture0);
	TexParameteri(); // 设置纹理参数

	// 加载并生成纹理对象
	GenerateTexImg("./container.jpg", GL_RGB);

	// 处理texture1
	glActiveTexture(GL_TEXTURE1); // 激活纹理单元1, 并绑定 texture1
	glBindTexture(GL_TEXTURE_2D, texture1);
	TexParameteri(); // 设置纹理参数

	// 加载并生成纹理对象
	GenerateTexImg("./awesomeface.png", GL_RGBA);

	// 在 GPU 中开辟指定类型的缓冲区, 用于存放 vertices indeies
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// 告知 GPU 如何解析顶点数据
	// 这一版去掉了纹顶点颜色
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), 0);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(0);
	//glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	glBindVertexArray(0); // 将vertexArray 从 OpenGL 当前上下文解绑

	//使 透明值 生效
	glEnable(GL_BLEND);//启用颜色混合操作功能
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);//设置颜色混合模式
	
	glEnable(GL_DEPTH_TEST);// 启用深度测试
	
	// 让箱子分别平移到 10 个不同的位置
	glm::vec3 cubePositions[] = {
		glm::vec3( 0.0f,  0.0f,  0.0f), 
		glm::vec3( 2.0f,  5.0f, -15.0f), 
		glm::vec3(-1.5f, -2.2f, -2.5f),  
		glm::vec3(-3.8f, -2.0f, -12.3f),  
		glm::vec3( 2.4f, -0.4f, -3.5f),  
		glm::vec3(-1.7f,  3.0f, -7.5f),  
		glm::vec3( 1.3f, -2.0f, -2.5f),  
		glm::vec3( 1.5f,  2.0f, -2.5f), 
		glm::vec3( 1.5f,  0.2f, -1.5f), 
		glm::vec3(-1.3f,  1.0f, -1.5f)  
	  };
	/*主循环*/
	glfwSwapInterval(1); // 设置前后缓冲区交换间隔，单位为帧
	while (!glfwWindowShouldClose(pWindow))
	{
		glfwPollEvents(); // 轮询 - glfw 与 窗口通信
		glClearColor(0.3f, 0.5f, 0.7f, 1.0f);//设置清除颜色缓冲区后要使用的颜色-纯色

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);// 清除颜色缓冲区 和 深度缓冲区
		
		glBindVertexArray(vertexArray); // 第二次绑定同一个 VAO 时，OpenGL 会使用这个 VAO 中记录的所有配置信息来进行绘制操作
		curTime = glfwGetTime();
		perFrameTime = curTime - lastTime;
		lastTime = curTime;
		// eyeFront = eyePos - pixelPos, 则 pixelPos = eyePos + eyeFront
		glm::mat4 view = glm::lookAt(eyePos, eyePos + eyeFront, eyeUp);
		glm::mat4 project = glm::perspective(glm::radians(fov), (float)wndWidth /  (float)wndHeight, 0.5f, 100.f);
		myShaderProgram.Use();	// 激活着色程序
		glUniformMatrix4fv(glGetUniformLocation(myShaderProgram.ID(), "view_"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(myShaderProgram.ID(), "project_"), 1, GL_FALSE, glm::value_ptr(project));
		myShaderProgram.SetUniform("texturer0", 0);// 将纹理单元0 传递给texturer0
		myShaderProgram.SetUniform("texturer1", 1);// 将纹理单元1 传递给texturer1
		
		for(int i = 0; i < 10; ++i)
		{
			// 模型变换矩阵
			// 实际是先沿 x 轴 再沿 y 轴旋转
			
			glm::mat4 model = glm::translate(glm::mat4(1.0f), cubePositions[i]);
			model = glm::translate(model, glm::vec3(0.f, 0.f, -1.f));
			//model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.f, 1.f, 0.f)); // 沿 y 轴随时间旋转
			model = glm::rotate(model, (float)glm::radians(-50.f), glm::vec3(1.f, 0.f, 0.f)); // 沿 x 轴旋转-50°
			glUniformMatrix4fv(glGetUniformLocation(myShaderProgram.ID(), "model_"), 1, GL_FALSE, glm::value_ptr(model));
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
		
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