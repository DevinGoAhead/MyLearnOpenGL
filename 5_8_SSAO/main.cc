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
	// 今天刚确认一件事,当纹理作为颜色缓冲时, format type 的类型并不重要, 他俩匹配的是输入数据的格式, 此时我们输入数据为 NULL
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
	// 示例代码选择的format 是 GLRGBA, 但是本练习使用的是单通道颜色, 因此,我感觉GL_RG就足够了吧
	uint gAlbedoSpecTex;
	setTexParameter(gAlbedoSpecTex, GL_TEXTURE_2D, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG, wndWidth, wndHeight, 0, GL_RG, GL_FLOAT, NULL);
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
		std::cout << "gFBO is not complete!" << std::endl;
	}
	
	// ssaoFBO
	uint ssaoFBO;
	glGenFramebuffers(1, &ssaoFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
	
	// 颜色缓冲-遮蔽因子
	uint ssaoTex;
	setTexParameter(ssaoTex, GL_TEXTURE_2D, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, wndWidth, wndHeight, 0, GL_RED, GL_FLOAT, NULL);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, ssaoTex, 0);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "ssaoFBO is not complete!" << std::endl;
	}

	// ssaoBlur
	uint ssaoBlurFBO;
	glGenFramebuffers(1, &ssaoBlurFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
	
	// 颜色缓冲-SSAO 模糊后的颜色, 仍然是单通道颜色
	uint ssaoBlurTex;
	setTexParameter(ssaoBlurTex, GL_TEXTURE_2D, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, wndWidth, wndHeight, 0, GL_RED, GL_FLOAT, NULL);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, ssaoBlurTex, 0);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "ssaoBlurFBO is not complete!" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	wxy::ShaderProgram shaderPrgmGBuf("./shader/gBuffer.vert", "./shader/gBuffer.frag");
	wxy::ShaderProgram shaderPrgmSSAO("./shader/quad.vert", "./shader/ssao.frag");
	//wxy::ShaderProgram shaderPrgmSSAOBlur("./shader/quad.vert", "./shader/ssaoBlur.frag");
	wxy::ShaderProgram shaderPrgmScreen("./shader/quad.vert", "./shader/shading.frag");
	
	wxy::Model nanosuit("../resources/objects/nanosuit/nanosuit.obj");

	// kernel
	std::random_device rd; // 随机数生成器, 用于提供随机数种子
	std::mt19937 gen(rd()); // mt19937 类型对象
	std::uniform_real_distribution<float> dist1(-1.f, 1.f);
	std::uniform_real_distribution<float> dist2(0.f, 1.f);

	std::vector<glm::vec3> samples;
	for(int i = 0; i < 64; ++i) {

		// 纹理空间, 表面正方向的一定范围的采样点
		glm::vec3 sample(dist1(gen), dist1(gen), dist2(gen));
		sample = glm::normalize(sample); // 归一化, 此时采样点在半球表面
		sample *= dist2(gen); // 随机分布在球体内部, 而不仅是表面

		// 接下来构造一个二次曲线, 使采样点更多的分布在核心附近
		float scale = i / 64.f; // x坐标
		// 太近的采样点没有意义, 直接使用 scale * scale, [0,0.1] 会分布许多无意义的点
		// 也不能使用clamp, 会有许多重复的点分布在0.1 的位置
		scale = std::lerp(0.1, 1.f, scale * scale);
		sample *= scale;

		samples.push_back(sample);
	}

	// noise: kernel random rotate
	// 当 kernel 确定后, 64个采样点就确定了, 整个模型都会按照这种采样点去采样, 会形成结构化噪点
	// 通过增加一个 nosize 并作为平铺纹理, 一定程度打破重复性, 并且增加了采样点覆盖的范围
	std::vector<glm::vec3> noises;
	for(int i = 0; i < 16; ++i) {noises.emplace_back(dist1(gen), dist1(gen), 0.f);} // 绕 z 轴旋转
	uint noiseTex;
	setTexParameter(noiseTex, GL_TEXTURE_2D, GL_REPEAT, GL_NEAREST, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, wndWidth, wndHeight, 0, GL_RED,  GL_FLOAT, noises.data());
	
	glm::vec3 lightPosition = glm::vec3(2.0, 4.0, -2.0);
    glm::vec3 lightColor = glm::vec3(0.2, 0.2, 0.7);

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

		//floor
		glm::mat4 model = glm::translate(glm::mat4(1.f),glm::vec3(0.f, -1.f, 0.f));
		model = glm::scale( model, glm::vec3(20.f, 1.f, 20.f));
		shaderPrgmGBuf.SetUniformv("uModel", model);
		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		
		// nanosuit
		model = glm::translate(glm::mat4(1.f),glm::vec3(0.f, 0.4f, 5.f));
		model = glm::rotate(model, glm::radians(90.f), glm::vec3(-1.f, 0.f, 0.f));
		model = glm::scale( model, glm::vec3(0.5));
		
		shaderPrgmGBuf.SetUniformv("uModel", model);
		nanosuit.Draw(shaderPrgmGBuf);

		// ssao
		glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
		glDisable(GL_DEPTH_TEST);
		glClearColor(0.1, 0.1,0.1, 1.f);
		glClear(GL_COLOR_BUFFER_BIT);

		shaderPrgmSSAO.Use();
		shaderPrgmSSAO.SetUniform("uWndWidth", wndWidth);
		shaderPrgmSSAO.SetUniform("uWndHeight", wndHeight);
		shaderPrgmSSAO.SetUniform("uRadius", 0.5f);
		shaderPrgmSSAO.SetUniform("uBias", 0.02f);
		shaderPrgmSSAO.SetUniformv("uSamples", 64, samples.data());
		shaderPrgmSSAO.SetUniformv("uProjection", projection);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gPositionTex);
		shaderPrgmScreen.SetUniform("uGBuffer.texturePosition", 0); // 位置

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gNormalTex);
		shaderPrgmScreen.SetUniform("uGBuffer.textureNormal", 1); // 法线

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, noiseTex);
		shaderPrgmSSAO.SetUniform("uTextureNoise", 2); // 核心随机旋转

		glBindVertexArray(quadVAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		// screen, 这里在观察空间渲染
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.1, 0.1,0.1, 1.f);
		glClear(GL_COLOR_BUFFER_BIT);

		shaderPrgmScreen.Use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gPositionTex);
		shaderPrgmScreen.SetUniform("uGBuffer.texturePosition", 0); // 位置

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gNormalTex);
		shaderPrgmScreen.SetUniform("uGBuffer.textureNormal", 1); // 法线

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, gAlbedoSpecTex);
		shaderPrgmScreen.SetUniform("uGBuffer.textureAlbedoSpec", 2); // 材质颜色 + 镜面反反射强度

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, ssaoTex);
		shaderPrgmScreen.SetUniform("uTextureOcclusion", 3); // 材质颜色 + 镜面反反射强度

		shaderPrgmScreen.SetUniformv("uLightPosition", glm::vec3(view * glm::vec4(lightPosition, 1.f))); // 光位置
		shaderPrgmScreen.SetUniformv("uLightColor", lightColor); // 光强度
		shaderPrgmScreen.SetUniform("uAttenuation.a", 0.032);
		shaderPrgmScreen.SetUniform("uAttenuation.b", 0.09);
		shaderPrgmScreen.SetUniform("uAttenuation.c", 1.f);

		glBindVertexArray(quadVAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		
		glfwSwapBuffers(pWindow); // 交换前后缓冲区
		glfwPollEvents(); // 轮询 - glfw 与 窗口通信
	}

	// 清理资源
	glfwDestroyWindow(pWindow); // 销毁窗口
	glfwTerminate();			// 终止GLFW
}