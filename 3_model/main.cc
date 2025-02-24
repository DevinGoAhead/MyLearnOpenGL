
#include "glad\glad.h"
#include "glfw\glfw3.h"

#include "MyClass\camera.hpp"
#include "model.hpp"
#include "global.h"

#include <filesystem>


int main()
{
	glfwSetErrorCallback(error_callback); // 设置回调, 捕获 glfw 错误
	glfwInit(); //GLFW 初始化

	// openGL 上下文配置
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 核心模式

	//创建窗口, 800 600 为初始尺寸
	GLFWwindow* pWindow = glfwCreateWindow(800, 600, "lighting", NULL, NULL);
	glfwMaximizeWindow(pWindow); // 最大化窗口
	glfwMakeContextCurrent(pWindow); // 设置 pWindow 窗口为当前上下文

	if(!gladLoadGLLoader(GLADloadproc(glfwGetProcAddress))) {
		std::cerr << "Initilize GLAD error" << std::endl;
		exit(1);
	}// 初始化 GLAD

	glfwGetFramebufferSize(pWindow, &wndWidth, &wndHeight); // 获取缓冲区尺寸
	glViewport(0, 0, wndWidth, wndHeight); // 设定视口尺寸
	
	glfwSetFramebufferSizeCallback(pWindow,viewport_size_callback);// 注册视口尺寸自动调整回调函数
	glfwSetKeyCallback(pWindow, key_callback);// 注册键盘动作处理回调函数
	glfwSetCursorPosCallback(pWindow, cursor_callback);// 注册光标捕捉函数
	glfwSetScrollCallback(pWindow, scroll_callback);// 注册滚轮捕捉函数
	// end of initialize

	stbi_set_flip_vertically_on_load(true); // 反转图像 y 轴
	wxy::ShaderProgram shaderProgram("../shader/boxVS.glsl", "../shader/boxFS.glsl");
	wxy::Model model(std::filesystem::absolute("../model/backpack/backpack.obj").string());

	glEnable(GL_BLEND);//启用颜色混合操作功能
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);//设置颜色混合模式
	glEnable(GL_DEPTH_TEST);// 启用深度测试

	// 主循环
	glfwSwapInterval(1); // 前后缓冲区交换间隔
	while(!glfwWindowShouldClose(pWindow))
	{
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);//设置清除颜色缓冲区后要使用的颜色-纯色
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 清楚颜色缓冲区和深度缓冲区

		curTime = glfwGetTime();
		perFrameTime = curTime - lastTime;
		lastTime = curTime;
		
		// 变换矩阵
		glm::mat4 modelTrans = glm::rotate(glm::mat4(1.f), glm::radians(40.f), glm::vec3(0.3f, 0.5f, 0.7f));
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 project = glm::perspective(glm::radians(camera.GetFov()), (float)wndWidth / (float)wndHeight, 0.1f, 100.f); 
		//draw
		shaderProgram.Use();

		glUniformMatrix4fv(glGetUniformLocation(shaderProgram.ID(), "model_"), 1, GL_FALSE, glm::value_ptr(modelTrans));
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram.ID(), "view_"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram.ID(), "project_"), 1, GL_FALSE, glm::value_ptr(project));

		model.Draw(shaderProgram);
		// 渲染
		glfwSwapBuffers(pWindow); // 交换前后缓冲区
		glfwPollEvents(); // 轮询 - glfw 与 窗口通信
	}
	glfwDestroyWindow(pWindow); // 销毁窗口
	glfwTerminate();			// 终止GLFW
}
	