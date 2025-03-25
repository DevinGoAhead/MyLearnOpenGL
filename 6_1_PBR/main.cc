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
		else if(nChannels == 1){
			internalFormat = GL_RED;
			format = GL_RED;
		}

		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, imageWidth, imageHeight, 0, format, GL_UNSIGNED_BYTE, pImageData); // 开辟内存, 存储图片
		glGenerateMipmap(GL_TEXTURE_2D);
		stbi_image_free(pImageData);
	};

	uint albedoTex;
	setTexParameter(albedoTex, GL_TEXTURE_2D, GL_REPEAT, GL_LINEAR,  GL_LINEAR);
	GenerateTex("../resources/textures/pbr/rusted_iron/albedo.png", true);

	uint normalTex;
	setTexParameter(normalTex, GL_TEXTURE_2D, GL_REPEAT, GL_LINEAR,  GL_LINEAR);
	GenerateTex("../resources/textures/pbr/rusted_iron/normal.png", false);

	uint metalnessTex;
	setTexParameter(metalnessTex, GL_TEXTURE_2D, GL_REPEAT, GL_LINEAR,  GL_LINEAR);
	GenerateTex("../resources/textures/pbr/rusted_iron/metallic.png", false);// 原始数据在线性空间

	uint roughnessTex;
	setTexParameter(roughnessTex, GL_TEXTURE_2D, GL_REPEAT, GL_LINEAR,  GL_LINEAR);
	GenerateTex("../resources/textures/pbr/rusted_iron/roughness.png", false); // 原始数据在线性空间

	uint aoTex;
	setTexParameter(aoTex, GL_TEXTURE_2D, GL_CLAMP_TO_EDGE, GL_LINEAR,  GL_LINEAR);
	GenerateTex("../resources/textures/pbr/rusted_iron/ao.png", true);

	wxy::Sphere sphere;
	wxy::ShaderProgram shaderPrgmPBR("./shader/pbr.vs", "./shader/pbr.fs");

	// lights
	// std::vector <glm::vec3> lightPositions {
	// 	{-10.0f,  10.0f, 10.0f},
	// 	{ 10.0f,  10.0f, 10.0f},
	// 	{-10.0f, -10.0f, 10.0f},
	// 	{ 10.0f, -10.0f, 10.0f}
	// };
	std::vector <glm::vec3> lightPositions { // 测试示例效果
		{0.f, 0.f, 10.0f},
		{0.f, 0.f, 10.0f},
		{0.f, 0.f, 10.0f},
		{0.f, 0.f, 10.0f}
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
		shaderPrgmPBR.SetUniformv("uF0", glm::vec3(0.04)); //基础反射率
		shaderPrgmPBR.SetUniformv("uCameraPosition", camera.GetPos());
		shaderPrgmPBR.SetUniformv("uLightColors", 4, lightColors.data());
		shaderPrgmPBR.SetUniformv("uLightPositions", 4, lightPositions.data());

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, albedoTex);
		shaderPrgmPBR.SetUniform("uAlbedo", 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, roughnessTex);
		shaderPrgmPBR.SetUniform("uRoughness", 1);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, metalnessTex);
		shaderPrgmPBR.SetUniform("uMetalness", 2);

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, aoTex);
		shaderPrgmPBR.SetUniform("uAO", 3);

		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, normalTex);
		shaderPrgmPBR.SetUniform("uNormal", 4);

		// draw sphere
		for(int iRow = 0; iRow < nrRows; ++iRow) {
			for(int iCol = 0; iCol < nrColumns; ++iCol) {
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