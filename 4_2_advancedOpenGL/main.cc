#include "global.h"

int main()
{
	glfwSetErrorCallback(error_callback); // 设置回调, 捕获 glfw 错误
	glfwInit(); //GLFW 初始化

	// openGL 上下文配置
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 核心模式

	//创建窗口, 800 600 为初始尺寸
	GLFWwindow* pWindow = glfwCreateWindow(800, 600, "lighting", NULL, NULL);
	glfwMaximizeWindow(pWindow); // 最大化窗口
	glfwMakeContextCurrent(pWindow); // 设置 pWindow 窗口为当前上下文

	if(!gladLoadGLLoader(GLADloadproc(glfwGetProcAddress))) {std::cerr << "Initilize GLAD error" << std::endl;}// 初始化 GLAD
	std::cout << glGetString(GL_VERSION);
	glfwGetFramebufferSize(pWindow, &wndWidth, &wndHeight); // 获取缓冲区尺寸
	glViewport(0, 0, wndWidth, wndHeight); // 设定视口尺寸
	
	glfwSetFramebufferSizeCallback(pWindow,viewport_size_callback);// 注册视口尺寸自动调整回调函数
	glfwSetKeyCallback(pWindow, key_callback);// 注册键盘动作处理回调函数
	glfwSetCursorPosCallback(pWindow, cursor_callback);// 注册光标捕捉函数
	glfwSetScrollCallback(pWindow, scroll_callback);// 注册滚轮捕捉函数
	
	// 顶点

	// 顶点着色器 location index, 属性元素分量, 类型, 是否标准化, 步距, 相对起点偏移
	auto Set3DVertAttrPtr = [](){
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0); // 顶点坐标
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float))); // 纹理
		glEnableVertexAttribArray(0); // 启用location 0
		glEnableVertexAttribArray(1); // 启用location 1
	};

	auto Set2DVertAttrPtr = [](){
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0); // 顶点坐标
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float))); // 纹理
		glEnableVertexAttribArray(0); // 启用location 0
		glEnableVertexAttribArray(1); // 启用location 1
	};

	// VBO VAO 只能是引用传递或指针传递
	auto SetVertices = [&](GLuint& VBO, GLuint& VAO, const std::vector<float>& Vertices, int dimension){
		glGenBuffers(1, &VBO); // 创建 Buffer 对象, 绑定到 ID VBO 上
		glBindBuffer(GL_ARRAY_BUFFER, VBO); // 将 VBO 绑定在 GL_ARRAY_BUFFER 上

		// 在 GPU 中开辟指定类型的缓冲区, 用于存放 vertices indeies
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * Vertices.size(), &Vertices[0], GL_STATIC_DRAW);

		glGenVertexArrays(1, &VAO); // 创建 vertexArray, 绑定到 ID VAO 上
		glBindVertexArray(VAO); // 绑定 VAO 到 openGL 上下文, 开始记录信息

		if(dimension == 2) {Set2DVertAttrPtr();}
		else if(dimension == 3) {Set3DVertAttrPtr();}

		glBindVertexArray(0); // 将 VAO 从 OpenGL 当前上下文解绑
	};

	// 顶点数据在 global.h 中
	GLuint cubeVBO, cubeVAO; // cube
	SetVertices(cubeVBO, cubeVAO, cubeVertices, 3);

	GLuint planeVBO, planeVAO; // plane
	SetVertices(planeVBO, planeVAO, planeVertices, 3);

	GLuint quadVBO, quadVAO; // quadrangle
	SetVertices(quadVBO, quadVAO,quadVertices, 2);

	// 纹理
	// 纹理参数设置，需要传递纹理环绕参数
	auto TexParameteri = [](GLint param){
		// 2D 纹理, ST 坐标, 超出边界重复
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, param);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, param);

		// 纹理映射缩放算法
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);//纹理若被放大, 则线性插值
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);//纹理若被缩小, 则三线性插值
	};

	// 加载并生成纹理图像
	auto GenerateTexImg = [](const char* pathName){
		int texWidth, texHeight, nRChannels;
		unsigned char* pImageData = stbi_load(pathName, &texWidth, &texHeight, &nRChannels, 0);
		if(!pImageData) {
			std::cerr << "STBI::ERROR::failed to load Image" << '\n';
			exit(1);
		}

		GLenum format;
		if(4 == nRChannels) {format = GL_RGBA;}
		else if(3 == nRChannels) {format = GL_RGB;}

		// 生成的纹理图像将保存在 glGenTextures 所生成的对象中
		glTexImage2D(GL_TEXTURE_2D, 0, format, texWidth, texHeight, 0, format, GL_UNSIGNED_BYTE, pImageData);
		glGenerateMipmap(GL_TEXTURE_2D);// 自动生成多级渐远纹理
		stbi_image_free(pImageData);// 释放图片资源
	};

	GLuint texture0;
	glGenTextures(1, &texture0); // 将纹理对象绑定到的 ID 绑定到texture0
	// 激活纹理单元0
	// 将 texture0 绑定到 纹理单元0 的 GL_TEXTURE_2D
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture0);
	TexParameteri(GL_REPEAT); // 设置纹理参数
	GenerateTexImg("../resources/textures/container2.png");// 加载并生成纹理对象

	GLuint texture1;
	glGenTextures(1, &texture1); // 将纹理对象绑定到的 ID 绑定到texture1
	// 激活纹理单元1, 并绑定 texture1
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture1);
	TexParameteri(GL_REPEAT); // 设置纹理参数
	GenerateTexImg("../resources/textures/marble.jpg");// 加载并生成纹理对象

	int framWidth, framHeight;
	glfwGetFramebufferSize(pWindow, &framWidth, &framHeight);
	// 创建自定义帧缓冲
	uint framBufObj;
	glGenFramebuffers(1, &framBufObj); // 创建帧缓冲对象, 绑定framBufObj
	glBindFramebuffer(GL_FRAMEBUFFER, framBufObj); // 绑定到可渲染, 可读取的帧缓冲目标上

	// 创建纹理缓冲, 用作颜色缓冲
	uint texBufObj;
	glGenTextures(1, &texBufObj); // 创建纹理对象, 绑定 texBufObj
	glBindTexture(GL_TEXTURE_2D, texBufObj); // 绑定到 2D纹理目标上
	// 一般不会设置越界的纹理坐标, 仅仅是恰好贴合即可
	// 纹理映射缩放算法, 不涉及相机距离问题, 因此也不关心 MipMap
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //纹理若被放大, 则线性插值
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //纹理若被缩小, 则选择邻近像素即可

	// 确定纹理缓冲的大小, 通常与视口大小匹配即可, 不填充图像, 仅仅是一个空的纹理, 后面将渲染结果保存在这里
	// format 根据需要设置
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, framWidth, framHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	// 绑定到帧缓冲
	// 目标帧缓冲类型, 当前绑定缓冲的作用(本例是颜色缓冲), 纹理类型, 
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texBufObj, 0);

	// 创建渲染缓冲, 用于模板缓冲和深度缓冲
	uint rendBufObj;
	glGenRenderbuffers(1, &rendBufObj); // 创建渲染缓冲, 绑定 rendBufObj
	glBindRenderbuffer(GL_FRAMEBUFFER, rendBufObj); // 绑定到目标GL_FRAMEBUFFER, 目前阶段, 仅 GL_FRAMEBUFFER 一个类型
	// 8位用作模板缓冲, 24位用作颜色缓冲, 同时确定缓冲区尺寸(大小)
	glRenderbufferStorage(GL_FRAMEBUFFER, GL_DEPTH24_STENCIL8,framWidth, framHeight);
	
	//参数含义与 glFramebufferTexture2D 类似
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rendBufObj);
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "GL::ERROR:: FrameBuffer is not complete" << std::endl;
	}
	
	// 将帧缓冲绑定为默认帧缓冲
	// 但是我感觉这里不仅没必要, 而且徒增麻烦
	glBindFramebuffer(GL_FRAMEBUFFER, 0); 

	wxy::ShaderProgram shaderPrgmScene("./shader/scene.vert", "./shader/scene.frag");
	wxy::ShaderProgram shaderPrgmScreen("./shader/screen.vert", "./shader/screen.frag");

	// 主循环
	glfwSwapInterval(1); // 前后缓冲区交换间隔
	while(!glfwWindowShouldClose(pWindow))
	{
		glBindFramebuffer(GL_FRAMEBUFFER, framBufObj); // 将自定义帧缓冲绑定到目标上
		glEnable(GL_DEPTH_TEST);// 启用深度测试, 因为后面要关闭深度测试, 因此需要将深度测试打开放到循环中
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);//设置清除颜色缓冲区后要使用的颜色-纯色
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 清楚颜色缓冲区, 深度缓冲区, 本例不需要模板缓冲

		curTime = glfwGetTime();
		perFrameTime = curTime - lastTime;
		lastTime = curTime;

		// 这里 绑定 GL_FRAMBUFFER 的还是自定义的帧缓冲, 所以渲染结果都会到自定义帧缓冲上
		shaderPrgmScene.Use(); // 绘制箱子和地板

		// 视图变换矩阵和投影变换矩阵是所有对象共用
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 project = glm::perspective(glm::radians(camera.GetFov()), (float)wndWidth / (float)wndHeight, 0.1f, 100.f); 
		shaderPrgmScene.SetUniformv("view_", 1, view);
		shaderPrgmScene.SetUniformv("project_", 1, project);
		
		// draw plane
		// texture
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture1);
		shaderPrgmScene.SetUniform("texturer0_",1);

		glBindVertexArray(planeVAO);
		shaderPrgmScene.SetUniformv("model_", 1, glm::mat4(1.f)); // 模型矩阵
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// draw cube
		// 纹理
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture0);
		shaderPrgmScene.SetUniform("texturer0_",0);

		glBindVertexArray(cubeVAO);
		for(const auto& position : cubePositions) {
			glm::mat4 cubeModel = glm::translate(glm::mat4(1.f), position);
			shaderPrgmScene.SetUniformv("model_", 1, cubeModel);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
		//以上, 自定义帧缓冲渲染完成

		// 映射到屏幕
		glBindFramebuffer(GL_FRAMEBUFFER, 0);// 将默认帧缓冲绑定到 GL_FRAMEBUFFER
		
		glDisable(GL_DEPTH_TEST); // 2D 不需要深度测试
		glClearColor(0.8f, 0.6f, 0.4f, 1.0f);//设置清除颜色缓冲区后要使用的颜色-纯色
		glClear(GL_COLOR_BUFFER_BIT); // 清楚颜色缓冲区

		shaderPrgmScreen.Use();
		// 纹理
		// 此时纹理缓冲的作用将和普通纹理对象相同, 需要激活纹理通道
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, texBufObj);
		shaderPrgmScreen.SetUniform("texturerScreen_",2);

		glBindVertexArray(quadVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		
		glfwSwapBuffers(pWindow); // 交换前后缓冲区
		glfwPollEvents(); // 轮询 - glfw 与 窗口通信
	}

	// 清理资源
	glfwDestroyWindow(pWindow); // 销毁窗口
	glfwTerminate();			// 终止GLFW
}