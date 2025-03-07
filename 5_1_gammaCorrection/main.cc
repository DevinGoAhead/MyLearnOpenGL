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
	auto SetVertices = [](GLuint& VBO, const std::vector<float>& vertices, auto& SetVAO){
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
		SetVAO();
	};

	// cube
	GLuint planeVBO, planeVAO;
	auto SetPlaneVAO = [&planeVAO](){
		glGenVertexArrays(1, &planeVAO);
		glBindVertexArray(planeVAO);
		
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0); // 顶点
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float))); // 法线
		glEnableVertexAttribArray(1);

		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float))); // 纹理
		glEnableVertexAttribArray(2);

		glBindVertexArray(0); //重置默认绑定
	};
	SetVertices(planeVBO, planeVertices, SetPlaneVAO);

	// 设置纹理
	// 创建纹理对象
	auto setTexParameter = [](GLuint& tex){
		glGenTextures(1, &tex);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex);
		// 设置纹理环绕参数
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		// 设置纹理映射(过滤)方式
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // 放大
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // 缩小
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

	GLuint texPlane;
	setTexParameter(texPlane);
	GenerateTex(false);

	GLuint texPlaneGammaCorr;
	setTexParameter(texPlaneGammaCorr);
	GenerateTex(true);

	std::vector<glm::vec3> lightPoses{
        glm::vec3(-5.0f, 0.0f, 0.0f),
        glm::vec3(-2.0f, 0.0f, 0.0f),
        glm::vec3(2.0f, 0.0f, 0.0f),
        glm::vec3(5.0f, 0.0f, 0.0f)
    };
    std::vector<glm::vec3> lightColors{
        glm::vec3(0.6f),
        glm::vec3(0.8f),
        glm::vec3(0.9f),
        glm::vec3(0.7f)
    };

	wxy::ShaderProgram shaderPrgmPlane("./shader/plane.vert", "./shader/plane.frag");

	// 主循环
	glEnable(GL_DEPTH_TEST);
	glfwSwapInterval(1); // 前后缓冲区交换间隔
	while(!glfwWindowShouldClose(pWindow)) {
		curTime = glfwGetTime();
		perFrameTime = curTime - lastTime;
		lastTime = curTime;

		glClearColor(0.1f, 0.1f,0.1f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// 变换矩阵
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 projection = glm::perspective(glm::radians(camera.GetFov()), (float)wndWidth / (float)wndHeight, 0.1f, 100.f);

		shaderPrgmPlane.Use();
		shaderPrgmPlane.SetUniformv("uModel", glm::mat4(1.f));
		shaderPrgmPlane.SetUniformv("uView", view);
		shaderPrgmPlane.SetUniformv("uProjection", projection);

		// 材质
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, isGamma ? texPlaneGammaCorr : texPlane);
		shaderPrgmPlane.SetUniform("textureDiffuse", 0);

		shaderPrgmPlane.SetUniformv("uCameraPos", camera.GetPos()); // 相机位置

		// 灯光
		shaderPrgmPlane.SetUniformv("uLightColors", 4, lightColors.data());
		shaderPrgmPlane.SetUniformv("uLightPoses", 4, lightPoses.data());

		shaderPrgmPlane.SetUniform("uGamma", isGamma); // 切换 是否 gamma 矫正
		//blinn ? std::cout << "blinn" <<'\n' : std::cout << "phong" <<'\n';

		glBindVertexArray(planeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glfwSwapBuffers(pWindow); // 交换前后缓冲区
		glfwPollEvents(); // 轮询 - glfw 与 窗口通信
	}

	// 清理资源
	glfwDestroyWindow(pWindow); // 销毁窗口
	glfwTerminate();			// 终止GLFW
}