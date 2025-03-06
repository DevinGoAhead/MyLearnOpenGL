#include "global.h"
#include <chrono>

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
	//std::cout << glGetString(GL_VERSION);
	glfwSetFramebufferSizeCallback(pWindow,viewport_size_callback);// 注册视口尺寸自动调整回调函数
	glfwMaximizeWindow(pWindow); // 最大化窗口
	
	// Wait until the size callback has been triggered at least once
	// auto start = std::chrono::high_resolution_clock::now();
    while (wndWidth == 0 || wndHeight == 0) {
        glfwPollEvents();  // Make sure GLFW is processing events
    }
	// auto end = std::chrono::high_resolution_clock::now();
	// std::chrono::duration<double> duration = end - start;
	// std::cout << "Time elapsed: " << duration.count() << " seconds" << std::endl; // 0.015 
	glfwSetKeyCallback(pWindow, key_callback);// 注册键盘动作处理回调函数
	glfwSetCursorPosCallback(pWindow, cursor_callback);// 注册光标捕捉函数
	glfwSetScrollCallback(pWindow, scroll_callback);// 注册滚轮捕捉函数
	
	// VBO VAO
	auto SetVertices = [](GLuint& VBO, const std::vector<float>& vertices, auto& SetVAO){
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
		SetVAO();
	};

	// cube
	GLuint cubeVBO, cubeVAO;
	auto SetCubeVAO = [&cubeVAO](){
		glGenVertexArrays(1, &cubeVAO);
		glBindVertexArray(cubeVAO);
		
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
		glEnableVertexAttribArray(0);
		glBindVertexArray(0); //重置默认绑定
	};
	SetVertices(cubeVBO, cubeVertices, SetCubeVAO);

	// screen
	GLuint screenVBO, screenVAO;
	auto SetScreenVAO = [&screenVAO](){
		glGenVertexArrays(1, &screenVAO);
		glBindVertexArray(screenVAO);
		
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
		glEnableVertexAttribArray(1);

		glBindVertexArray(0); //重置默认绑定
	};
	SetVertices(screenVBO, quadVertices, SetScreenVAO);
	// MSAA FBO
	GLuint msFBO;
	glGenFramebuffers(1, &msFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, msFBO);

	// texture for msFBO
	GLuint msTex;
	glGenTextures(1, &msTex);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msTex);
	// 采样行为由硬件固定, 不支持任何纹理过滤参数
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB, wndWidth, wndHeight, GL_TRUE);//设置 msTex 的尺寸
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, msTex, 0); // 绑定到帧缓冲作为ie颜色缓冲附件

	// RBO for  msFBO
	GLuint msRBO;
	glGenRenderbuffers(1, &msRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, msRBO);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, wndWidth, wndHeight);// 设置 RBO 的尺寸(物理像素)
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH24_STENCIL8, GL_RENDERBUFFER, msRBO);// 绑定到帧缓冲, 作为深度和模板缓冲附件
	
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "GL::ERROR:: FrameBuffer is not complete" << std::endl;
	}

	// intermediary FBO
	GLuint postProcessFBO;
	glGenFramebuffers(1, &postProcessFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, postProcessFBO); // 绑定到可读可写的 目标 上
	
	// Texture for intermediary FBO
	GLuint postProcessTex;
	glGenTextures(1, &postProcessTex);
	glBindTexture(GL_TEXTURE_2D, postProcessTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, wndWidth, wndHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);// 设置 TBO 的尺寸(物理像素)
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, postProcessTex, 0); // 绑定到帧缓冲, 作为颜色缓冲附件

	// 作为后处理, 模板测试和深度测试在 msFBO 中已经完成了,这里仅对颜色做处理即可, 因而不需要 RBO
	// // RBO for intermediary FBO
	// GLuint postProcessRBO;
	// glGenRenderbuffers(1, &postProcessRBO);
	// glBindRenderbuffer(GL_RENDERBUFFER, postProcessRBO);

	// // 设置RBO 的内部格式和尺寸(物理像素)
	// glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, wndWidth, wndHeight);
	// glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH24_STENCIL8, GL_RENDERBUFFER, postProcessRBO);

	// 检查报错
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "GL::ERROR:: FrameBuffer is not complete" << std::endl;
	}
	
	wxy::ShaderProgram shaderPrgmCube("./shader/cube.vert", "./shader/cube.frag");
	wxy::ShaderProgram shaderPrgmScreen("./shader/screen.vert", "./shader/screen.frag");
	// 主循环
	glfwSwapInterval(1); // 前后缓冲区交换间隔
	while(!glfwWindowShouldClose(pWindow)) {
		curTime = glfwGetTime();
		perFrameTime = curTime - lastTime;
		lastTime = curTime;

		glBindFramebuffer(GL_FRAMEBUFFER, msFBO);
		glEnable(GL_DEPTH_TEST);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);//设置清除颜色缓冲区后要使用的颜色-纯色
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 projection = glm::perspective(glm::radians(camera.GetFov()), (float)wndWidth / (float)wndHeight, 0.1f, 100.f);

		shaderPrgmCube.Use();
		shaderPrgmCube.SetUniformv("uModel", glm::rotate(glm::mat4(1.f), glm::radians(45.f), glm::vec3(0.f, 1.f, 0.f)));
		shaderPrgmCube.SetUniformv("uView", view);
		shaderPrgmCube.SetUniformv("uProjection", projection);

		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		//end of msFBO

		// transfer to postProcessFBO
		glBindFramebuffer(GL_READ_FRAMEBUFFER, msFBO); // 读取
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, postProcessFBO); // 写入
		glBlitFramebuffer(0, 0, wndWidth, wndHeight, 0, 0, wndWidth, wndHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
		// end of postProcessFBO

		// draw to screen
		glBindFramebuffer(GL_FRAMEBUFFER, 0); // 绑定至默认帧缓冲
		glDisable(GL_DEPTH_TEST);
		glClearColor(0.6f, 0.6f, 0.6f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT);

		shaderPrgmScreen.Use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, postProcessTex);
		shaderPrgmScreen.SetUniform("uTexture", 0);
		glBindVertexArray(screenVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glfwSwapBuffers(pWindow); // 交换前后缓冲区
		glfwPollEvents(); // 轮询 - glfw 与 窗口通信
	}

	// 清理资源
	glfwDestroyWindow(pWindow); // 销毁窗口
	glfwTerminate();			// 终止GLFW
}