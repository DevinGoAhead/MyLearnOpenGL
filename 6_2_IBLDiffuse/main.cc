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

	// env 纹理
	uint envHDRTex;
	setTexParameter(envHDRTex, GL_TEXTURE_2D, GL_CLAMP_TO_EDGE, GL_LINEAR,  GL_LINEAR);
	// 加载纹理
	stbi_set_flip_vertically_on_load(true);
	int imageWidth, imageHeight, nChannels;
	float* pImageData = stbi_loadf("../resources/textures/hdr/newport_loft.hdr", &imageWidth, &imageHeight, &nChannels, 0);
	if(!pImageData) {
		std::cerr << "failed to load Image" << '\n';
		exit(1);
	}
	// 这里不是很明白, 作者在文章中说明了 radiance HDR 的第4个通道是 指数
	// 但这里加载却使用了 GL_RGB, 而不是 GL_RGBA?
	glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB16F, imageWidth, imageHeight, 0, GL_RGB, GL_FLOAT, pImageData); // 开辟内存, 存储图片
	stbi_image_free(pImageData);

	//envCubeFBO, 将环境贴图从等距柱状投影图采样到 cube map 中
	uint envCubeFBO;
	int ecFBOWidth = 1024, ecFBOHeight = ecFBOWidth;
	glGenFramebuffers(1, &envCubeFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, envCubeFBO);
	
	uint envCubeTex;
	setTexParameter(envCubeTex, GL_TEXTURE_CUBE_MAP, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR);
	for(int i = 0; i < 6; ++i) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, ecFBOWidth, ecFBOHeight, 0, GL_RGB, GL_FLOAT, NULL);
	}
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, envCubeTex, 0);
	// 去掉了RBO, 不需要深度测试

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "envCubeFBO is not complete!" << std::endl;
	}


	// 采样环境立方体贴图, 预计算 irradiance, 存储到
	uint irradianceCubeFBO;
	int icFBOWidth = 32, icFBOHeight = icFBOWidth;
	glGenFramebuffers(1, & irradianceCubeFBO);
	glBindFramebuffer(GL_FRAMEBUFFER,  irradianceCubeFBO);
	
	uint irradianceCubeTex;
	setTexParameter(irradianceCubeTex, GL_TEXTURE_CUBE_MAP, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR);
	for(int i = 0; i < 6; ++i) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, icFBOWidth, icFBOHeight, 0, GL_RGB, GL_FLOAT, NULL);
	}
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, irradianceCubeTex, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "irradianceCubeFBO is not complete!" << std::endl;
	}

	// 为cube 构造投影矩阵, 同时为每个面构造视图矩阵, 摄像机一直在中心, 旋转切换方向
	// 两个立方体纹理共用
	glm::mat4 ecProjection = glm::perspective(glm::radians(90.f), (float)ecFBOWidth / ecFBOHeight, 0.1f, 10.f); // fov-90 确保能看到所有内容
	std::vector<glm::mat4> ecViews {
		glm::lookAt(glm::vec3(0.f), glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f, -1.f, 0.f)),
		glm::lookAt(glm::vec3(0.f), glm::vec3(-1.f, 0.f, 0.f), glm::vec3(0.f, -1.f, 0.f)),
		glm::lookAt(glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.f, 0.f, 1.f)),
		glm::lookAt(glm::vec3(0.f), glm::vec3(0.f, -1.f, 0.f), glm::vec3(0.f, 0.f, -1.f)),
		glm::lookAt(glm::vec3(0.f), glm::vec3(0.f, 0.f, 1.f), glm::vec3(0.f, -1.f, 0.f)),
		glm::lookAt(glm::vec3(0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec3(0.f, -1.f, 0.f))
	};

	wxy::Sphere sphere;
	wxy::ShaderProgram shaderPrgmPBR("./shader/pbr.vs", "./shader/pbr.fs");
	wxy::ShaderProgram shaderPrgmEqRectToCube("./shader/cube.vs", "./shader/eqRectToCube.fs");
	wxy::ShaderProgram shaderPrgmIrradianceMap("./shader/cube.vs", "./shader/irradianceMap.fs");
	wxy::ShaderProgram shaderPrgmCubeMap("./shader/cubeMap.vs", "./shader/cubeMap.fs");

	//lights
	std::vector <glm::vec3> lightPositions {
		{-10.0f,  10.0f, 10.0f},
		{ 10.0f,  10.0f, 10.0f},
		{-10.0f, -10.0f, 10.0f},
		{ 10.0f, -10.0f, 10.0f}
	};
	std::vector <glm::vec3> lightColors {
		{150.0f, 150.0f, 150.0f},
		{150.0f, 150.0f, 150.0f},
		{150.0f, 150.0f, 150.0f},
		{150.0f, 150.0f, 150.0f}
	};
	int nrRows = 7;
	int nrColumns = 7;
	float spacing = 2.5;

	// 主循环
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glfwSwapInterval(1); // 前后缓冲区交换间隔
	while(!glfwWindowShouldClose(pWindow)) {
		curTime = glfwGetTime();
		perFrameTime = curTime - lastTime;
		lastTime = curTime;

		glClearColor(0.1, 0.1,0.1, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 projection = glm::perspective(glm::radians(camera.GetFov()), (float)wndWidth / wndHeight, 0.1f, 100.f);

		// create cube map textue
		glBindFramebuffer(GL_FRAMEBUFFER, envCubeFBO);
		glViewport(0, 0, ecFBOWidth, ecFBOHeight);

		shaderPrgmEqRectToCube.Use();
		shaderPrgmEqRectToCube.SetUniformv("uProjection", ecProjection);
		
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, envHDRTex);
		shaderPrgmEqRectToCube.SetUniform("uTextureEqRect", 0);
		
		glBindVertexArray(cubeVAO);
		for(int i = 0; i < 6; ++i) {
			shaderPrgmEqRectToCube.SetUniformv("uView", ecViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubeTex, 0);
			glClear(GL_COLOR_BUFFER_BIT);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		// convolution irradiance
		glBindFramebuffer(GL_FRAMEBUFFER,  irradianceCubeFBO);
		glViewport(0, 0, icFBOWidth, icFBOHeight);

		shaderPrgmIrradianceMap.Use();
		shaderPrgmIrradianceMap.SetUniformv("uProjection", ecProjection);
		
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, envCubeTex);
		shaderPrgmIrradianceMap.SetUniform("uTextureCube", 0);
		
		glBindVertexArray(cubeVAO);
		for(int i = 0; i < 6; ++i) {
			shaderPrgmEqRectToCube.SetUniformv("uView", ecViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceCubeTex, 0);
			glClear(GL_COLOR_BUFFER_BIT);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		// draw background
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, wndWidth, wndHeight);

		shaderPrgmCubeMap.Use();
		shaderPrgmCubeMap.SetUniformv("uView", view);
		shaderPrgmCubeMap.SetUniformv("uProjection", projection);
		
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, envCubeTex);
		shaderPrgmCubeMap.SetUniform("uTextureCube", 0);
		
		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// draw ball
		shaderPrgmPBR.Use();
		shaderPrgmPBR.SetUniformv("uView", view);
		shaderPrgmPBR.SetUniformv("uProjection", projection);
		shaderPrgmPBR.SetUniformv("uF0", glm::vec3(0.04)); //基础反射率
		shaderPrgmPBR.SetUniformv("uCameraPosition", camera.GetPos());
		shaderPrgmPBR.SetUniformv("uLightColors", 4, lightColors.data());
		shaderPrgmPBR.SetUniformv("uLightPositions", 4, lightPositions.data());

		shaderPrgmPBR.SetUniform("uAO", 0.9f);
		shaderPrgmPBR.SetUniformv("uAlbedo", glm::vec3(0.5f, 0.0f, 0.0f));

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceCubeTex);
		shaderPrgmPBR.SetUniform("uTextureCube", 0);

		// draw sphere
		for(int iRow = 0; iRow < nrRows; ++iRow) {
			shaderPrgmPBR.SetUniform("uMetalness", (float)iRow / nrRows);
			for(int iCol = 0; iCol < nrColumns; ++iCol) {
				//glm::mat4 model = glm::translate(glm::mat4(1.f), glm::vec3(iRow, iCol, 0) * spacing);
				glm::mat4 model = glm::translate(glm::mat4(1.f), 
							glm::vec3( iCol - nrColumns / 2.f, iRow - nrRows / 2.f, -2 / spacing) * spacing); // 示例代码布局, 由中心向四周扩散
				shaderPrgmPBR.SetUniform("uRoughness", glm::clamp((float)iCol / nrColumns, 0.05f, 1.f));
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