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
	auto setTexParameter = [](GLuint& tex, GLenum target, GLint wrappingParam, GLint magFilterParam, GLint minFilterParam){
		glGenTextures(1, &tex);
		//glActiveTexture(GL_TEXTURE0);
		glBindTexture(target, tex);
		// 设置纹理环绕参数
		glTexParameteri(target, GL_TEXTURE_WRAP_S, wrappingParam);
		glTexParameteri(target, GL_TEXTURE_WRAP_T, wrappingParam);
		if(target == GL_TEXTURE_CUBE_MAP) glTexParameteri(target, GL_TEXTURE_WRAP_R, wrappingParam);

		if(wrappingParam == GL_CLAMP_TO_BORDER) {
			GLfloat borderColor[] ={1.0f, 1.0f, 1.0f, 1.0f};
			glTexParameterfv(target, GL_TEXTURE_BORDER_COLOR, borderColor);
		}
		

		// 设置纹理映射(过滤)方式
		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, magFilterParam); // 放大
		glTexParameteri(target, GL_TEXTURE_MIN_FILTER, minFilterParam); // 缩小
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

	GLuint planeTexGammaCorr;
	setTexParameter(planeTexGammaCorr, GL_TEXTURE_2D, GL_REPEAT, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR);
	GenerateTex(true);

	// FBO for shadow mapping
	GLuint depthMapFBO;
	glGenFramebuffers(1,&depthMapFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);

	// Texture for depthMapFBO
	GLuint depthCubeMapTex;
	setTexParameter(depthCubeMapTex, GL_TEXTURE_CUBE_MAP, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST); // 深度纹理默认不支持 GL_REPEAT and GL_LINEAR
	int depthWidth = 1024, depthHeight = 1024;
	for(int i = 0; i < 6; ++i) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, depthWidth, depthHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);// 为纹理开辟内存
	}
	// glFramebufferTexture2D 仅支持 2D 纹理
	// glFramebufferTexture 后期引入汉书, 可自动推断纹理类型
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubeMapTex, 0);
	
	// 不需要颜色缓冲
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "GL::ERROR: FBO is not complete" << std::endl;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0); // 绑定至默认帧缓冲
	
	wxy::ShaderProgram shaderPrgmDepthMap("./shader/modelDepthMap.vert", "./shader/modelDepthMap.geom", "./shader/modelDepthMap.frag");
	wxy::ShaderProgram shaderPrgmModel("./shader/model.vert", "./shader/model.frag");

	// 光坐标系视图和投影矩阵
	glm::vec3 lightPos = glm::vec3(0.f);
	float near = 1.f, far = 25.f;
	float depthAspect = (float)depthWidth / depthHeight;
	glm::mat4 lightProj = glm::perspective(glm::radians(90.f), depthAspect, near, far);
	std::vector<glm::mat4> lightProjViewMats;

	// lightPos + center, 确保看的方向是相对光源的, 而不是世界坐标中的绝对坐标
	lightProjViewMats.push_back(lightProj * glm::lookAt(lightPos, lightPos + glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f, -1.f, 0.f))); // right, up -Y
	lightProjViewMats.push_back(lightProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.f, 0.f, 0.f), glm::vec3(0.f, -1.f, 0.f))); // left, up -Y
	lightProjViewMats.push_back(lightProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.f, 0.f, 1.f))); // top, up +Z
	lightProjViewMats.push_back(lightProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.f, -1.f, 0.f), glm::vec3(0.f, 0.f, -1.f))); // bot, up -Z
	lightProjViewMats.push_back(lightProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.f, 0.f, 1.f), glm::vec3(0.f, -1.f, 0.f))); // back, up -Y
	lightProjViewMats.push_back(lightProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.f, 0.f, -1.f), glm::vec3(0.f, -1.f, 0.f))); // front, up -Y
	
	std::vector<glm::mat4> cubeModelMats;
	//1
	glm::mat4 model =  glm::translate(glm::mat4(1.f), glm::vec3(4.0f, -3.5f, 0.0));
	cubeModelMats.push_back(model);
	//2
	model = glm::translate(glm::mat4(1.f), glm::vec3(2.0f, 3.0f, 1.0));
    model = glm::scale(model, glm::vec3(1.5f));
	cubeModelMats.push_back(model);
	//3
	model = glm::translate(glm::mat4(1.f), glm::vec3(-3.0f, -1.f, 0.0));
	cubeModelMats.push_back(model);
	//4
	model = glm::translate(glm::mat4(1.f), glm::vec3(-1.5f, 1.f, 1.5f));
	cubeModelMats.push_back(model);
	//5
	model = glm::translate(glm::mat4(1.f), glm::vec3(-1.5f, 2.f, -3.f));
    model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
    model = glm::scale(model, glm::vec3(1.5f));
	cubeModelMats.push_back(model);
	
	// 主循环
	glEnable(GL_DEPTH_TEST);
	glfwSwapInterval(1); // 前后缓冲区交换间隔
	while(!glfwWindowShouldClose(pWindow)) {
		curTime = glfwGetTime();
		perFrameTime = curTime - lastTime;
		lastTime = curTime;
		
		glClearColor(0.1, 0.1,0.1, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		// depth map
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glViewport(0, 0, depthWidth, depthHeight);
		glClear(GL_DEPTH_BUFFER_BIT);

		shaderPrgmDepthMap.Use();
		shaderPrgmDepthMap.SetUniformv("uLightProjViewMats", 6, lightProjViewMats.data());
		shaderPrgmDepthMap.SetUniformv("uLightPos", lightPos);
		shaderPrgmDepthMap.SetUniform("uFar", far);
		
		// depth map of room cube
		glBindVertexArray(cubeVAO);
		shaderPrgmDepthMap.SetUniformv("uModel", glm::scale(glm::mat4(1.f), glm::vec3(10.f))); // 放大 10 倍
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// depth map of member cubes
		glBindVertexArray(cubeVAO);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT); // 记录背面的深度, 在消除 shadow acen 的同时还不会造成 peter panning 问题, 实测效果很差
		for(const auto& mat : cubeModelMats) {
			shaderPrgmDepthMap.SetUniformv("uModel", mat);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
		glDisable(GL_CULL_FACE);
		
		// model
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, wndWidth, wndHeight);
		glClearColor(0.1, 0.1,0.1, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// 视图和投影变换矩阵
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 projection = glm::perspective(glm::radians(camera.GetFov()), (float)wndWidth / wndHeight, 0.1f, 100.f);

		shaderPrgmModel.Use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, planeTexGammaCorr);
		shaderPrgmModel.SetUniform("uMaterial.textureDiffuse0", 0); // 材质纹理

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubeMapTex);
		shaderPrgmModel.SetUniform("uTextureDepthMap", 1); // 深度纹理
		shaderPrgmModel.SetUniformv("uCameraPos", camera.GetPos()); // 相机位置
		shaderPrgmModel.SetUniformv("uLightPos", lightPos); // 光位置

		// matrix
		shaderPrgmModel.SetUniformv("uView", view);
		shaderPrgmModel.SetUniformv("uProjection", projection);
		//shaderPrgmModel.SetUniformv("uProjViewAtLgt", projViewAtLgt);
		
		// model of room cube
		glBindVertexArray(cubeVAO);
		shaderPrgmModel.SetUniformv("uModel", glm::scale(glm::mat4(1.f), glm::vec3(10.f)));
		shaderPrgmModel.SetUniform("uRNormal", 1);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		
		// model of menber cube
		//shaderPrgmModel.SetUniform("uIs3D", );
		shaderPrgmModel.SetUniform("uRNormal", 0);
		for(const auto& mat : cubeModelMats) {
			shaderPrgmModel.SetUniformv("uModel", mat);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
		glfwSwapBuffers(pWindow); // 交换前后缓冲区
		glfwPollEvents(); // 轮询 - glfw 与 窗口通信
	}

	// 清理资源
	glfwDestroyWindow(pWindow); // 销毁窗口
	glfwTerminate();			// 终止GLFW
}