#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include <filesystem>

#include "global.h"
#include "model.hpp"

int main() {
	//GLFWwindow* pWindow = Initialize(); // openGL GLAD GLFW 初始化
	glfwSetErrorCallback(error_callback); // 设置回调, 捕获 glfw 错误
	glfwInit(); //GLFW 初始化

	// openGL 上下文配置
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 核心模式
 
	//创建窗口, 800 600 为初始尺寸
	GLFWwindow* pWindow = glfwCreateWindow(800, 600, "Model Importing", NULL, NULL);
	glfwMaximizeWindow(pWindow); // 最大化窗口
	glfwMakeContextCurrent(pWindow); // 设置 pWindow 窗口为当前上下文

	if(!gladLoadGLLoader(GLADloadproc(glfwGetProcAddress))) {std::cerr << "Initilize GLAD error" << std::endl;}// 初始化 GLAD

	glfwGetFramebufferSize(pWindow, &wndWidth, &wndHeight); // 获取缓冲区尺寸
	glViewport(0, 0, wndWidth, wndHeight); // 设定视口尺寸
	
	glfwSetFramebufferSizeCallback(pWindow,viewport_size_callback);// 注册视口尺寸自动调整回调函数
	glfwSetKeyCallback(pWindow, key_callback);// 注册键盘动作处理回调函数
	glfwSetCursorPosCallback(pWindow, cursor_callback);// 注册光标捕捉函数
	glfwSetScrollCallback(pWindow, scroll_callback);// 注册滚轮捕捉函数
	
	stbi_set_flip_vertically_on_load(true);
	wxy::ShaderProgram shaderProgram("./shader/model.vert", "./shader/model.frag"); //着色器

	wxy::Model modeler(std::filesystem::absolute("./model/backpack/backpack.obj").string());

	glEnable(GL_DEPTH_TEST);
	glfwSwapInterval(1);

	while(!glfwWindowShouldClose(pWindow)) {
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);//设置清除颜色缓冲区后要使用的颜色-纯色
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 清楚颜色缓冲区和深度缓冲区

		curTime = glfwGetTime();
		perFrameTime = curTime - lastTime;
		lastTime = curTime;

		glm::mat4 model(glm::mat4(1.f));
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 project = glm::perspective(glm::radians(camera.GetFov()), (float)wndWidth / (float)wndHeight, 0.1f, 100.f);
		
		// set uniform
		shaderProgram.Use();
		shaderProgram.SetUniformv("model_", 1, model);
		shaderProgram.SetUniformv("view_", 1, view);
		shaderProgram.SetUniformv("project_", 1, project);

		// draw
		modeler.Draw(shaderProgram);
		glfwSwapBuffers(pWindow);
        glfwPollEvents();
	}

	glfwDestroyWindow(pWindow); // 销毁窗口
	glfwTerminate();			// 终止GLFW
}
	