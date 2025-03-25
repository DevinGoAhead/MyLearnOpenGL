#include "global.h"

int main()
{
	glfwSetErrorCallback(error_callback); // 设置回调, 捕获 glfw 错误
	glfwInit(); //GLFW 初始化

	// openGL 上下文配置
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 核心模式
	glfwWindowHint(GLFW_SAMPLES, 4); // 指定多重采样的采样个数

	//创建窗口, 800 600 为初始尺寸
	GLFWwindow* pWindow = glfwCreateWindow(800, 600, "OpenGL", NULL, NULL);
	glfwMakeContextCurrent(pWindow); // 设置 pWindow 窗口为当前上下文
	if(!gladLoadGLLoader(GLADloadproc(glfwGetProcAddress))) {std::cerr << "Initilize GLAD error" << std::endl;}// 初始化 GLAD
	
	glfwSetFramebufferSizeCallback(pWindow,viewport_size_callback);// 先注册视口尺寸自动调整回调函数
	glfwMaximizeWindow(pWindow); // 最大化窗口
	
	// Wait until the size callback has been triggered at least once
    while (wndWidth == 0 || wndHeight == 0) {
        glfwPollEvents();  // Make sure GLFW is processing events
    }

	glfwSetKeyCallback(pWindow, key_callback);// 注册键盘动作处理回调函数
	glfwSetCursorPosCallback(pWindow, cursor_callback);// 注册光标捕捉函数
	glfwSetScrollCallback(pWindow, scroll_callback);// 注册滚轮捕捉函数

	wxy::Sphere sphere;
	wxy::ShaderProgram shaderPrgmPBR("./shader/pbr.vs", "./shader/pbr.fs");

	// lights
	std::vector <glm::vec3> lightPositions {
		{-10.0f,  10.0f, 10.0f},
		{ 10.0f,  10.0f, 10.0f},
		{-10.0f, -10.0f, 10.0f},
		{ 10.0f, -10.0f, 10.0f}
	};
	std::vector <glm::vec3> lightColors {
		{300.0f, 300.0f, 300.0f},
		{300.0f, 300.0f, 300.0f},
		{300.0f, 300.0f, 300.0f},
		{300.0f, 300.0f, 300.0f}
	};
	int nrRows = 7;
	int nrColumns = 7;
	float spacing = 2.5;

	// 主循环
	glEnable(GL_DEPTH_TEST);
	glfwSwapInterval(1); // 前后缓冲区交换间隔
	while(!glfwWindowShouldClose(pWindow)) {
		curTime = glfwGetTime();
		perFrameTime = curTime - lastTime;
		lastTime = curTime;

		glClearColor(0.1, 0.1,0.1, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 projection = glm::perspective(glm::radians(camera.GetFov()), (float)wndWidth / wndHeight, 0.1f, 100.f);
		
		shaderPrgmPBR.Use();
		shaderPrgmPBR.SetUniformv("uView", view);
		shaderPrgmPBR.SetUniformv("uProjection", projection);
		shaderPrgmPBR.SetUniform("uAO", 0.9);
		shaderPrgmPBR.SetUniformv("uAlbedo", glm::vec3(0.5f, 0.0f, 0.0f));
		shaderPrgmPBR.SetUniformv("uF0", glm::vec3(0.04)); //基础反射率
		shaderPrgmPBR.SetUniformv("uCameraPosition", camera.GetPos());
		shaderPrgmPBR.SetUniformv("uLightColors", 4, lightColors.data());

		// 光源可视化
		for(int i = 0; i < 4; ++i) {
			glm::vec3 newPos = lightPositions[i] + glm::vec3(sin(curTime * 5.f)  * 5.f, 0.f, 0.f);
			shaderPrgmPBR.SetUniformv(("uLightPositions[" + std::to_string(i) + "]").c_str(), newPos);
			glm::mat4 model = glm::translate(glm::mat4(1.f), newPos);
			shaderPrgmPBR.SetUniformv("uModel", model);
			sphere.Draw();
		}
		// draw sphere
		for(int iRow = 0; iRow < nrRows; ++iRow) {
			shaderPrgmPBR.SetUniform("uMetalness", (float)iRow / nrRows);
			for(int iCol = 0; iCol < nrColumns; ++iCol) {
				shaderPrgmPBR.SetUniform("uRoughness", glm::clamp((float)iCol / nrColumns, 0.05f, 1.f));
				//glm::mat4 model = glm::translate(glm::mat4(1.f), glm::vec3(iRow, iCol, 0) * spacing);
				glm::mat4 model = glm::translate(glm::mat4(1.f), 
							glm::vec3(iRow - nrRows / 2.f, iCol - nrColumns / 2.f, 0) * spacing); // 示例代码布局, 由中心向四周扩散
				shaderPrgmPBR.SetUniformv("uModel", model);
				sphere.Draw();
			}
		}


		glfwSwapBuffers(pWindow); // 交换前后缓冲区
		glfwPollEvents(); // 轮询 - glfw 与 窗口通信
	}

	// 清理资源
	glfwDestroyWindow(pWindow); // 销毁窗口
	glfwTerminate();			// 终止GLFW
}