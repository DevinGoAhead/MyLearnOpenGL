#include "global.h"

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

	if(!gladLoadGLLoader(GLADloadproc(glfwGetProcAddress))) {std::cerr << "Initilize GLAD error" << std::endl;}// 初始化 GLAD
	//std::cout << glGetString(GL_VERSION);
	glfwGetFramebufferSize(pWindow, &wndWidth, &wndHeight); // 获取缓冲区尺寸
	glViewport(0, 0, wndWidth, wndHeight); // 设定视口尺寸
	
	glfwSetFramebufferSizeCallback(pWindow,viewport_size_callback);// 注册视口尺寸自动调整回调函数
	glfwSetKeyCallback(pWindow, key_callback);// 注册键盘动作处理回调函数
	glfwSetCursorPosCallback(pWindow, cursor_callback);// 注册光标捕捉函数
	glfwSetScrollCallback(pWindow, scroll_callback);// 注册滚轮捕捉函数
	
	wxy::ShaderProgram shaderPrgm("./shader/model.vert", "./shader/model.geom", "./shader/model.frag");
	wxy::Model nanosuit("../resources/objects/nanosuit/nanosuit.obj");
	
	glm::vec3 light(1.f, 1.f, 1.f);

	glm::vec3 posLight(50.f);

	glEnable(GL_DEPTH);
	// 主循环
	glfwSwapInterval(1); // 前后缓冲区交换间隔
	while(!glfwWindowShouldClose(pWindow))
	{
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);//设置清除颜色缓冲区后要使用的颜色-纯色
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 清楚颜色缓冲区

		curTime = glfwGetTime();
		perFrameTime = curTime - lastTime;
		lastTime = curTime;

		shaderPrgm.Use();
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 projection = glm::perspective(glm::radians(camera.GetFov()), (float)wndWidth / (float)wndHeight, 0.1f, 100.f);
		glm::mat4 modelNanosuit =  glm::translate(glm::mat4(1.f), glm::vec3(0.f, -2.f, 0.f));
		modelNanosuit = glm::scale(modelNanosuit, glm::vec3(0.5f));
		modelNanosuit = glm::rotate(modelNanosuit, glm::radians(5.f), glm::vec3(0.f, -1.f, 0.f));

		shaderPrgm.SetUniformv("model_", 1, modelNanosuit);
		shaderPrgm.SetUniformv("view_", 1, view);
		shaderPrgm.SetUniformv("projection_", 1, projection);
		shaderPrgm.SetUniform("time_", curTime);

		// light
		shaderPrgm.SetUniformv("light_.ambient", 1, glm::vec3(1.f));
		shaderPrgm.SetUniformv("light_.diffuse", 1, glm::vec3(1.f));
		shaderPrgm.SetUniformv("light_.specular", 1, glm::vec3(1.f));
		shaderPrgm.SetUniformv("cameraPos", 1, camera.GetPos());

		nanosuit.Draw(shaderPrgm);
		
		glfwSwapBuffers(pWindow); // 交换前后缓冲区
		glfwPollEvents(); // 轮询 - glfw 与 窗口通信
	}

	// 清理资源
	glfwDestroyWindow(pWindow); // 销毁窗口
	glfwTerminate();			// 终止GLFW
}