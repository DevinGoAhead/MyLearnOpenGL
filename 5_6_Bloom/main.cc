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
	
	// VBO VAO
	auto SetVertices = [](GLuint& VBO, const std::vector<float>& vertices){
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
	};

	// cube
	GLuint cubeVBO,cubeVAO;
	SetVertices(cubeVBO,cubeVertices);
	auto SetVAOPosNorTex = [](GLuint& VAO){
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
	SetVAOPosNorTex(cubeVAO);

	// quadrangle
	GLuint quadVBO, quadVAO;
	SetVertices(quadVBO, quadVertices);
	auto SetVAOPosTex = [](GLuint& VAO){
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);
		
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0); // 顶点
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float))); // 纹理
		glEnableVertexAttribArray(1);

		glBindVertexArray(0); //重置默认绑定
	};
	SetVAOPosTex(quadVAO);

	// 设置纹理
	// 创建纹理对象
	auto setTexParameter = [](GLuint& tex, GLenum target, GLint wrappingParam, GLint magFilterParam, GLint minFilterParam){
		glGenTextures(1, &tex);
		glBindTexture(target, tex);
		// 设置纹理环绕参数
		if(wrappingParam != -1) {
			glTexParameteri(target, GL_TEXTURE_WRAP_S, wrappingParam);
			glTexParameteri(target, GL_TEXTURE_WRAP_T, wrappingParam);
			if(target == GL_TEXTURE_CUBE_MAP) glTexParameteri(target, GL_TEXTURE_WRAP_R, wrappingParam);

			if(wrappingParam == GL_CLAMP_TO_BORDER) {
				GLfloat borderColor[] ={1.0f, 1.0f, 1.0f, 1.0f};
				glTexParameterfv(target, GL_TEXTURE_BORDER_COLOR, borderColor);
			}
		}
		// 设置纹理映射(过滤)方式
		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, magFilterParam); // 放大
		glTexParameteri(target, GL_TEXTURE_MIN_FILTER, minFilterParam); // 缩小
	};
	
	auto GenerateTex = [](const char* pathName, bool isGammaCorrect){
		// 加载纹理
		int imageWidth, imageHeight, nChannels;
		unsigned char* pImageData = stbi_load(pathName, &imageWidth, &imageHeight, &nChannels, 0);
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

	GLuint woodTex;
	setTexParameter(woodTex, GL_TEXTURE_2D, GL_REPEAT, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR);
	GenerateTex("../resources/textures/wood.png", true);

	GLuint containerTex;
	setTexParameter(containerTex, GL_TEXTURE_2D, GL_REPEAT, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR);
	GenerateTex("../resources/textures/container2.png", true);

	// 自定义 HDR 帧缓冲
	uint hdrFBO;
	glGenFramebuffers(1, &hdrFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
	
	uint hdrTex[2]; // 颜色缓冲, 1 个处理结果hdr, 一个用于 Blur 使用
	for(int i = 0; i < 2; ++i) {
		setTexParameter(hdrTex[i], GL_TEXTURE_2D, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, wndWidth, wndHeight, 0, GL_RGBA, GL_FLOAT, NULL); // 为纹理缓冲开辟内存
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, hdrTex[i], 0);
	}
	GLenum attachment[2]{GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
	glDrawBuffers(2, attachment); // 指定 片段着色器写入到哪些颜色缓冲(附件)
	
	uint hdrRBO; //深度缓冲
	glGenRenderbuffers(1, &hdrRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, hdrRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, wndWidth, wndHeight); // 为深度缓冲开辟内存
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, hdrRBO);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "Framebuffer is not complete!" << std::endl;
	}

	// 自定义 2个用于 Blur 的帧缓冲
	uint pingpongFBO[2];
	uint pingpongTex[2];
	for(int i = 0; i < 2; ++i) {
		glGenFramebuffers(1, &pingpongFBO[i]);
		glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
		
		setTexParameter(pingpongTex[i], GL_TEXTURE_2D, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, wndWidth, wndHeight, 0, GL_RGBA, GL_FLOAT, NULL); // 为纹理缓冲开辟内存
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, pingpongTex[i], 0);
		// 不需要深度缓冲
		if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			std::cout << "Framebuffer is not complete!" << std::endl;
		}
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	wxy::ShaderProgram shaderPrgmObjectHDR("./shader/hdr.vert", "./shader/hdrObject.frag");
	wxy::ShaderProgram shaderPrgmLightHDR("./shader/hdr.vert", "./shader/hdrLight.frag");
	wxy::ShaderProgram shaderPrgmBlur("./shader/quad.vert", "./shader/quadBlur.frag");
	wxy::ShaderProgram shaderPrgmScreen("./shader/quad.vert", "./shader/quadBlend.frag");

	// lighting info
    // positions
    std::vector<glm::vec3> lightPositions;
    lightPositions.push_back(glm::vec3( 0.0f, 0.5f,  1.5f));
    lightPositions.push_back(glm::vec3(-4.0f, 0.5f, -3.0f));
    lightPositions.push_back(glm::vec3( 3.0f, 0.5f,  1.0f));
    lightPositions.push_back(glm::vec3(-0.8f, 2.4f, -1.0f));
    // colors
    std::vector<glm::vec3> lightColors;
    lightColors.push_back(glm::vec3(5.0f,   5.0f,  5.0f));
    lightColors.push_back(glm::vec3(10.0f,  0.0f,  0.0f));
    lightColors.push_back(glm::vec3(0.0f,   0.0f,  15.0f));
    lightColors.push_back(glm::vec3(0.0f,   5.0f,  0.0f));
	
	//cube model matrix
	std::vector<glm::mat4> cubeModelMats;
	glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.5f, 0.0));
	model = glm::scale(model, glm::vec3(0.5f));
	cubeModelMats.push_back(model);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 0.0f, 1.0));
	model = glm::scale(model, glm::vec3(0.5f));
	cubeModelMats.push_back(model);
	
	model = glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, -1.0f, 2.0));
	model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
	cubeModelMats.push_back(model);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 2.7f, 4.0));
	model = glm::rotate(model, glm::radians(23.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
	model = glm::scale(model, glm::vec3(1.25));
	cubeModelMats.push_back(model);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(-2.0f, 1.0f, -3.0));
	model = glm::rotate(model, glm::radians(124.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
	cubeModelMats.push_back(model);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(-3.0f, 0.0f, 0.0));
	model = glm::scale(model, glm::vec3(0.5f));
	cubeModelMats.push_back(model);

	// 主循环
	glfwSwapInterval(1); // 前后缓冲区交换间隔
	while(!glfwWindowShouldClose(pWindow)) {
		curTime = glfwGetTime();
		perFrameTime = curTime - lastTime;
		lastTime = curTime;

		// HDR
		glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
		glEnable(GL_DEPTH_TEST);
		glClearColor(0.1, 0.1,0.1, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		// 视图和投影变换矩阵
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 projection = glm::perspective(glm::radians(camera.GetFov()), (float)wndWidth / wndHeight, 0.1f, 100.f);
		
		shaderPrgmObjectHDR.Use();
		shaderPrgmObjectHDR.SetUniformv("uView", view);
		shaderPrgmObjectHDR.SetUniformv("uProjection", projection);
		//shaderPrgmObjectHDR.SetUniformv("uCameraPos", camera.GetPos()); // 相机位置
		shaderPrgmObjectHDR.SetUniformv("uLightPositions", 4, lightPositions.data()); // 光位置
		shaderPrgmObjectHDR.SetUniformv("ulightColors", 4, lightColors.data()); // 光强度

		// floor
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, woodTex);
		shaderPrgmObjectHDR.SetUniform("uMaterial.textureDiffuse0", 0); // 材质纹理

		glm::mat4 modelFloor =  glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0));
		modelFloor  = glm::scale(modelFloor, glm::vec3(12.5f, 0.5f, 12.5f));
		shaderPrgmObjectHDR.SetUniformv("uModel", modelFloor);
		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		
		//cube
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, containerTex);
		shaderPrgmObjectHDR.SetUniform("uMaterial.textureDiffuse0", 0); // 材质纹理
		for(const auto& cubeModeMat : cubeModelMats) {
			shaderPrgmObjectHDR.SetUniformv("uModel", cubeModeMat);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		//light
		shaderPrgmLightHDR.Use();
		shaderPrgmLightHDR.SetUniformv("uView", view);
		shaderPrgmLightHDR.SetUniformv("uProjection", projection);
		for(int i = 0; i < 4; ++i) {
			glm::mat4 modelLight = glm::translate(glm::mat4(1.f), lightPositions[i]);
			modelLight = glm::scale(modelLight, glm::vec3(0.25));
			
			shaderPrgmLightHDR.SetUniformv("uModel", modelLight);
			shaderPrgmLightHDR.SetUniformv("uLightColor", lightColors[i]);

			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		// Blur
		// 半径为 5 就足够大了, 我尝试生成半径为 6 的高斯数据,发现最边缘的权重的数量级已经在10^-9了, 没什么必要了
		glDisable(GL_DEPTH_TEST);
		float weight[5] = {0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162};
	
		shaderPrgmBlur.Use();
		shaderPrgmBlur.SetUniformv("uWeight", 5, weight);
		glBindVertexArray(quadVAO);
		
		for(int i = 0; i < 15; ++i) { // 来回 N 次, 总共 N / 2 次高斯模糊, 最终结果在 pingpongTex[1]
			int cur = i % 2;
			glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[cur]);
			
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, i == 0 ? hdrTex[1] : pingpongTex[1 - cur]);
			shaderPrgmBlur.SetUniform("uTextureBright", 0);

			shaderPrgmBlur.SetUniform("uHorizontal", 1 - cur);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		}

		// Screen, Blend
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.1, 0.1,0.1, 1.f);
		glClear(GL_COLOR_BUFFER_BIT);
		
		shaderPrgmScreen.Use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, hdrTex[0]);
		shaderPrgmScreen.SetUniform("uHDRTexture", 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D,  pingpongTex[1]);
		shaderPrgmScreen.SetUniform("uBlurTexture", 1);

		shaderPrgmScreen.SetUniform("uExposure", exposure);
		
		glBindVertexArray(quadVAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		
		glfwSwapBuffers(pWindow); // 交换前后缓冲区
		glfwPollEvents(); // 轮询 - glfw 与 窗口通信
	}

	// 清理资源
	glfwDestroyWindow(pWindow); // 销毁窗口
	glfwTerminate();			// 终止GLFW
}