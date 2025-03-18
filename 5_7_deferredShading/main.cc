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

	// 自定义 Geometry Buffer
	uint gFBO;
	glGenFramebuffers(1, &gFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, gFBO);
	
	// 颜色缓冲-顶点位置
	uint gPositionTex;
	// 示例代码中没有设置纹理环绕方式, 而且, 通常也确实不需要设置, 但是我查了一些资料, 一些现代引擎中选择设置为 GL_CLAMP_TO_EDGE 避免越界
	setTexParameter(gPositionTex, GL_TEXTURE_2D, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, wndWidth, wndHeight, 0, GL_RGB, GL_FLOAT, NULL);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, gPositionTex, 0);

	// 颜色缓冲-法线
	uint gNormalTex;
	setTexParameter(gNormalTex, GL_TEXTURE_2D, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, wndWidth, wndHeight, 0, GL_RGB, GL_FLOAT, NULL);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, gNormalTex, 0);

	// 颜色缓冲-Albedo + specular intensity
	// 第 4 个分量存储 specular intensity
	// 为什么没有存储镜面反射纹理的颜色? 我实际测试了一下, 也打开镜面反射纹理的图片看了一下, 发现他的纹理是单通道灰色的, 也就是说仅存储了反射强度, 没有 Albedo
	uint gAlbedoSpecTex;
	setTexParameter(gAlbedoSpecTex, GL_TEXTURE_2D, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, wndWidth, wndHeight, 0, GL_RGBA, GL_FLOAT, NULL);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, gAlbedoSpecTex, 0);

	// 指定 片段着色器写入到哪些颜色缓冲(附件)
	GLenum attachment[3]{GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
	glDrawBuffers(3, attachment);
	
	uint gRBO; //深度缓冲
	glGenRenderbuffers(1, &gRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, gRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, wndWidth, wndHeight); // 为深度缓冲开辟内存
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, gRBO);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "Framebuffer is not complete!" << std::endl;
	}
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	wxy::ShaderProgram shaderPrgmGBuf("./shader/gBuffer.vert", "./shader/gBuffer.frag");
	wxy::ShaderProgram shaderPrgmLightCube("./shader/lightCube.vert", "./shader/lightCube.frag");
	wxy::ShaderProgram shaderPrgmScreen("./shader/deferredShading.vert", "./shader/deferredShading.frag");
	
	wxy::Model nanosuit("../resources/objects/nanosuit/nanosuit.obj");

	// lighting info
	std::random_device rd; // 随机数生成器, 用于提供随机数种子
	std::mt19937 gen(rd()); // mt19937 类型对象
	std::uniform_real_distribution<float> dist1(-5.f, 5.f);
	std::uniform_real_distribution<float> dist2(0.4f, 0.8f);

	const int lightNr = 128;
    std::vector<glm::vec3> lightPositions;
	std::vector<glm::vec3> lightColors;
	for(int i = 0; i < lightNr; ++i){
		lightPositions.emplace_back(dist1(gen), dist1(gen), dist1(gen));
		lightColors.emplace_back(dist2(gen), dist2(gen), dist2(gen));
	}

	// 让模型做一个随即旋转
	int size = objectPositions.size();
	std::vector<float> rotates;
	while(size--) {rotates.push_back(glm::radians(dist1(gen) * 5.f));}

	// 主循环
	glfwSwapInterval(1); // 前后缓冲区交换间隔
	while(!glfwWindowShouldClose(pWindow)) {
		curTime = glfwGetTime();
		perFrameTime = curTime - lastTime;
		lastTime = curTime;

		// g-buffer
		glBindFramebuffer(GL_FRAMEBUFFER, gFBO);
		glEnable(GL_DEPTH_TEST);
		glClearColor(0.1, 0.1,0.1, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 projection = glm::perspective(glm::radians(camera.GetFov()), (float)wndWidth / wndHeight, 0.1f, 100.f);
		
		shaderPrgmGBuf.Use();
		shaderPrgmGBuf.SetUniformv("uView", view);
		shaderPrgmGBuf.SetUniformv("uProjection", projection);
		auto iter = rotates.begin();
		for(const auto& objectPosition : objectPositions) {
			glm::mat4 objectModel = glm::translate(glm::mat4(1.f), objectPosition);
			objectModel = glm::scale(objectModel, glm::vec3(0.25f));
			objectModel = glm::rotate(objectModel, *iter++, glm::vec3(0.f, 1.f, 0.f));
			shaderPrgmGBuf.SetUniformv("uModel", objectModel);
			nanosuit.Draw(shaderPrgmGBuf);
		}
		
		// deferred shading
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDisable(GL_DEPTH_TEST);
		glClearColor(0.1, 0.1,0.1, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shaderPrgmScreen.Use();
		shaderPrgmScreen.SetUniformv("uLightPositions", lightNr, lightPositions.data()); // 光位置
		shaderPrgmScreen.SetUniformv("uLightColors", lightNr, lightColors.data()); // 光强度
		shaderPrgmScreen.SetUniformv("uCameraPos", camera.GetPos());

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gPositionTex);
		shaderPrgmScreen.SetUniform("uGBuffer.texturePosition", 0); // 法线

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gNormalTex);
		shaderPrgmScreen.SetUniform("uGBuffer.textureNormal", 1); // 法线

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, gAlbedoSpecTex);
		shaderPrgmScreen.SetUniform("uGBuffer.textureAlbedoSpec", 2); // 材质颜色 + 镜面反反射强度

		glBindVertexArray(quadVAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glEnable(GL_DEPTH_TEST);
		shaderPrgmLightCube.Use();
		shaderPrgmLightCube.SetUniformv("uView", view);
		shaderPrgmLightCube.SetUniformv("uProjection", projection);
	
		for(int i = 0; i < lightNr; ++i) {
			shaderPrgmLightCube.SetUniformv("uLightColor", lightColors[i]);

			glm::mat4 lightModel = glm::translate(glm::mat4(1.f), lightPositions[i]);
			lightModel = glm::scale(lightModel, glm::vec3(0.125f));
			shaderPrgmLightCube.SetUniformv("uModel", lightModel);

			glBindVertexArray(cubeVAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
		glfwSwapBuffers(pWindow); // 交换前后缓冲区
		glfwPollEvents(); // 轮询 - glfw 与 窗口通信
	}

	// 清理资源
	glfwDestroyWindow(pWindow); // 销毁窗口
	glfwTerminate();			// 终止GLFW
}