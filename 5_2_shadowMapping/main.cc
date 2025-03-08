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
	
	glfwSetFramebufferSizeCallback(pWindow,viewport_size_callback);// 先注册视口尺寸自动调整回调函数
	glfwMaximizeWindow(pWindow); // 最大化窗口
	
	// Wait until the size callback has been triggered at least once
    while (wndWidth == 0 || wndHeight == 0) {
        glfwPollEvents();  // Make sure GLFW is processing events
    }

	glfwSetKeyCallback(pWindow, key_callback);// 注册键盘动作处理回调函数
	glfwSetCursorPosCallback(pWindow, cursor_callback);// 注册光标捕捉函数
	glfwSetScrollCallback(pWindow, scroll_callback);// 注册滚轮捕捉函数
	
	// VBO VAO
	auto SetVertices = [](GLuint& VBO, const std::vector<float>& vertices){
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
	};
	
	auto SetVAOVerNorTex = [](GLuint& VAO){
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);
		
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0); // 顶点
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float))); // 法线
		glEnableVertexAttribArray(1);

		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float))); // 纹理
		glEnableVertexAttribArray(2);

		glBindVertexArray(0); //重置默认绑定
	};

	// plane
	GLuint planeVBO, planeVAO;
	SetVertices(planeVBO, planeVertices);
	SetVAOVerNorTex(planeVAO);

	// cube
	GLuint cubeVBO, cubeVAO;
	SetVertices(cubeVBO, cubeVertices);
	SetVAOVerNorTex(cubeVAO);

	// screen
	GLuint screenVBO, screenVAO;
	SetVertices(screenVBO, screenVertices);
	// SetVAOScreen
	{
		glGenVertexArrays(1, &screenVAO);
		glBindVertexArray(screenVAO);
		
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0); // 顶点
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float))); // 纹理
		glEnableVertexAttribArray(1);

		glBindVertexArray(0); //重置默认绑定
	}

	// 设置纹理
	// 创建纹理对象
	auto setTexParameter = [](GLuint& tex, GLint wrappingParam, GLint magFilterParam, GLint minFilterParam){
		glGenTextures(1, &tex);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex);
		// 设置纹理环绕参数
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrappingParam);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrappingParam);

		// 设置纹理映射(过滤)方式
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilterParam); // 放大
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilterParam); // 缩小
	};
	
	auto GenerateTex = [](bool isGammaCorrect){
		// 加载纹理
		int imageWidth, imageHeight, nChannels;
		unsigned char* pImageData = stbi_load("../resources/textures/wood.png", &imageWidth, &imageHeight, &nChannels, 0);
		if(!pImageData) 
		{
			std::cerr << "failed to load Image" << '\n';
			exit(1);
		}

		GLint internalFormat;
		GLint format;
		
		if(nChannels == 4) {
			internalFormat = isGammaCorrect ?  GL_SRGB_ALPHA : GL_RGBA;
			format = GL_RGBA;
		}
		else if(nChannels == 3){
			internalFormat = isGammaCorrect ?  GL_SRGB : GL_RGB;
			format = GL_RGB;
		}

		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, imageWidth, imageHeight, 0, format, GL_UNSIGNED_BYTE, pImageData); // 开辟内存, 存储图片
		glGenerateMipmap(GL_TEXTURE_2D);
		stbi_image_free(pImageData);
	};

	GLuint texPlaneGammaCorr;
	setTexParameter(texPlaneGammaCorr, GL_REPEAT, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR);
	GenerateTex(true);

	// FBO for shadow mapping
	GLuint depthMapFBO;
	glGenFramebuffers(1,&depthMapFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);

	// Texture for depthMapFBO
	GLuint depthMapTex;
	setTexParameter(depthMapTex, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST); // 深度纹理默认不支持 GL_REPEAT GL_LINEAR

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);// 为纹理开辟内存
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTex, 0);
	
	// 不需要颜色缓冲
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "GL::ERROR: FBO is not complete" << std::endl;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0); // 绑定至默认帧缓冲
	
	wxy::ShaderProgram shaderPrgmDepthMap("./shader/modelDepthMap.vert", "./shader/modelDepthMap.frag");
	wxy::ShaderProgram shaderPrgmScreen("./shader/screen.vert", "./shader/screen.frag");

	glm::vec3 lightPos = glm::vec3{-2.f, 4.f, -1.f};
	
	std::vector<glm::mat4> cubeModelMats(3);
	//1
	glm::mat4 model =  glm::translate(glm::mat4(1.f), glm::vec3(0.0f, 1.5f, 0.0));
    model = glm::scale(model, glm::vec3(0.5f));
	cubeModelMats[0] = model;
	//2
	model = glm::translate(glm::mat4(1.f), glm::vec3(2.0f, 0.0f, 1.0));
    model = glm::scale(model, glm::vec3(0.5f));
	cubeModelMats[1] = model;
	//3
	model = glm::translate(glm::mat4(1.f), glm::vec3(-1.0f, 0.0f, 2.0));
    model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
    model = glm::scale(model, glm::vec3(0.25));
	cubeModelMats[2] = model;
	
	// 主循环
	glfwSwapInterval(1); // 前后缓冲区交换间隔
	while(!glfwWindowShouldClose(pWindow)) {
		curTime = glfwGetTime();
		perFrameTime = curTime - lastTime;
		lastTime = curTime;
		
		glClearColor(0.1, 0.1,0.1, 1.f);
		glClear(GL_COLOR_BUFFER_BIT);
		
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glEnable(GL_DEPTH_TEST);
		glClear(GL_DEPTH_BUFFER_BIT);

		// 变换矩阵, 以"光源坐标系"为参照
		glm::mat4 viewAtLight = glm::lookAt(lightPos, glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f)); // Up: 竖直向上

		// 这里暂先使用正教投影
		float orthoSize = 10.0f;
		glm::mat4 projectionAtLight = glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, 1.f, 7.5f); // left right bottom top near far

		shaderPrgmDepthMap.Use();
		shaderPrgmDepthMap.SetUniformv("uModel", glm::mat4(1.f));
		shaderPrgmDepthMap.SetUniformv("uView", viewAtLight);
		shaderPrgmDepthMap.SetUniformv("uProjection",projectionAtLight);
		
		glViewport(0, 0, 1024, 1024);
		glBindVertexArray(planeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glBindVertexArray(cubeVAO);
		for(const auto& mat : cubeModelMats) {
			shaderPrgmDepthMap.SetUniformv("uModel", mat);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		// sample to screen
		glViewport(0, 0, wndWidth, wndHeight);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDisable(GL_DEPTH_TEST);
		glClearColor(0.1, 0.1,0.1, 1.f);
		glClear(GL_COLOR_BUFFER_BIT);

		shaderPrgmScreen.Use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthMapTex);
		shaderPrgmScreen.SetUniform("uScreenTexture", 0);

		glBindVertexArray(screenVAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); // 本例用了4个顶点

		glfwSwapBuffers(pWindow); // 交换前后缓冲区
		glfwPollEvents(); // 轮询 - glfw 与 窗口通信
	}

	// 清理资源
	glfwDestroyWindow(pWindow); // 销毁窗口
	glfwTerminate();			// 终止GLFW
}