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
	//std::cout << glGetString(GL_VERSION);
	glfwGetFramebufferSize(pWindow, &wndWidth, &wndHeight); // 获取缓冲区尺寸
	glViewport(0, 0, wndWidth, wndHeight); // 设定视口尺寸
	
	glfwSetFramebufferSizeCallback(pWindow,viewport_size_callback);// 注册视口尺寸自动调整回调函数
	glfwSetKeyCallback(pWindow, key_callback);// 注册键盘动作处理回调函数
	glfwSetCursorPosCallback(pWindow, cursor_callback);// 注册光标捕捉函数
	glfwSetScrollCallback(pWindow, scroll_callback);// 注册滚轮捕捉函数
	
	// 顶点
	// VBO VAO 只能是引用传递或指针传递
	auto SetVertices = [](GLuint& VBO, GLuint& VAO, const std::vector<float>& Vertices, std::function<void(void)> VertAttrPtr){
		glGenBuffers(1, &VBO); // 创建 Buffer 对象, 绑定到 ID VBO 上
		glBindBuffer(GL_ARRAY_BUFFER, VBO); // 将 VBO 绑定在 GL_ARRAY_BUFFER 上

		// 在 GPU 中开辟指定类型的缓冲区, 用于存放 vertices indeies
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * Vertices.size(), &Vertices[0], GL_STATIC_DRAW);

		glGenVertexArrays(1, &VAO); // 创建 vertexArray, 绑定到 ID VAO 上
		glBindVertexArray(VAO); // 绑定 VAO 到 openGL 上下文, 开始记录信息

		VertAttrPtr();
	};

	// 顶点数据在 global.h 中

	//skyBox
	GLuint skyBoxVBO, skyBoxVAO;
	// 顶点着色器 location index, 属性元素分量, 类型, 是否标准化, 步距, 相对起点偏移
	std::function<void(void)> SkyBoxVertAttrPtr = [](){
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0); // 顶点坐标
		glEnableVertexAttribArray(0); // 启用location 0

		glBindVertexArray(0); // 将 VAO 从 OpenGL 当前上下文解绑
	};
	SetVertices(skyBoxVBO, skyBoxVAO, skyBoxVertices, SkyBoxVertAttrPtr);

	// cube
	GLuint cubeVBO, cubeVAO; 
	std::function<void(void)> CubeVertAttrPtr = [](){
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0); // 顶点坐标
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float))); // 法线
		glEnableVertexAttribArray(0); // 启用location 0
		glEnableVertexAttribArray(1); // 启用location 1

		glBindVertexArray(0); // 将 VAO 从 OpenGL 当前上下文解绑
	};
	SetVertices(cubeVBO, cubeVAO, cubeVertices, CubeVertAttrPtr);

	// 纹理

	// 纹理路径
	std::vector<std::string> skyBoxTexPaths{
		"../resources/textures/skybox/right.jpg",
		"../resources/textures/skybox/left.jpg",
		"../resources/textures/skybox/top.jpg",
		"../resources/textures/skybox/bottom.jpg",
		"../resources/textures/skybox/front.jpg",
		"../resources/textures/skybox/back.jpg",
	};

	// 纹理参数设置，需要传递纹理类型, 纹理环绕参数
	auto SetTexParai = [](GLuint& TextureID, GLenum target, GLint param){
		glGenTextures(1, &TextureID); // 将纹理对象绑定到的 ID 

		// 激活纹理单元0
		// 将 skyBoxTexID 绑定到 纹理单元0 的 GL_TEXTURE_CUBE_MAP 目标上
		glBindTexture(target, TextureID);
		glTexParameteri(target, GL_TEXTURE_WRAP_S, param);
		glTexParameteri(target, GL_TEXTURE_WRAP_T, param);
		glTexParameteri(target, GL_TEXTURE_WRAP_R, param);

		// 纹理映射缩放算法
		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	};

	// 加载并生成纹理图像
	auto GenerateTexImg = [](GLenum target, const char* pathName){
		int texWidth, texHeight, nRChannels;
		unsigned char* pImageData = stbi_load(pathName, &texWidth, &texHeight, &nRChannels, 0);
		if(!pImageData) {
			std::cerr << "STBI::ERROR::failed to load Image" << '\n';
			exit(1);
		}

		GLenum format;
		if(4 == nRChannels) {format = GL_RGBA;}
		else if(3 == nRChannels) {format = GL_RGB;}

		glTexImage2D(target, 0, format, texWidth, texHeight, 0, format, GL_UNSIGNED_BYTE, pImageData);
		stbi_image_free(pImageData);// 释放图片资源
	};

	// 生成纹理对象
	GLuint skyBoxTexID;
	glActiveTexture(GL_TEXTURE0);
	SetTexParai(skyBoxTexID, GL_TEXTURE_CUBE_MAP, GL_CLAMP_TO_EDGE); // 设置纹理参数
	// 加载纹理
	for(int i = 0; i < 6; ++i) {
		//enum 是依次递增的
		GenerateTexImg(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, skyBoxTexPaths[i].c_str());// 加载并生成纹理对象
	}

	wxy::ShaderProgram shaderPrgmCube("./shader/cube.vert", "./shader/cube.frag");
	wxy::ShaderProgram shaderPrgmSkyBox("./shader/skyBox.vert", "./shader/skyBox.frag");

	glEnable(GL_DEPTH_TEST);// 启用深度测试

	// 主循环
	glfwSwapInterval(1); // 前后缓冲区交换间隔
	while(!glfwWindowShouldClose(pWindow))
	{
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);//设置清除颜色缓冲区后要使用的颜色-纯色
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 清楚颜色缓冲区, 深度缓冲区, 本例不需要模板缓冲

		curTime = glfwGetTime();
		perFrameTime = curTime - lastTime;
		lastTime = curTime;

		// 视图变换矩阵和投影变换矩阵
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 projection = glm::perspective(glm::radians(camera.GetFov()), (float)wndWidth / (float)wndHeight, 0.1f, 100.f); 
		
		// draw the cube
		shaderPrgmCube.Use();
		glm::mat4 modelCube = glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.f, 0.f));
		modelCube = glm::rotate(modelCube, glm::radians(45.f), glm::vec3(0.f, 1.f, 0.f));
		shaderPrgmCube.SetUniformv("model_", 1, modelCube);
		shaderPrgmCube.SetUniformv("view_", 1, view);
		shaderPrgmCube.SetUniformv("projection_", 1, projection);
		shaderPrgmCube.SetUniform("cameraPos_", camera.GetPos());

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyBoxTexID);
		shaderPrgmCube.SetUniform("texerSkyBox_", 0);

		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0); // 解绑, 恢复默认

		// draw the skyBox at the end
		shaderPrgmSkyBox.Use();

		glm::mat4 viewSkyBox = glm::mat4(glm::mat3(view)); // 去掉平移部分
		shaderPrgmSkyBox.SetUniformv("view_", 1, viewSkyBox);
		shaderPrgmSkyBox.SetUniformv("projection_", 1, projection);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyBoxTexID);
		shaderPrgmSkyBox.SetUniform("texerSkyBox_", 0);

		glDepthFunc(GL_LEQUAL); // 因为天空盒的 深度值与 深度缓冲初始化值相等, 因此需要 <= 缓冲值即通过测试, 否则无法显示天空盒
		glBindVertexArray(skyBoxVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glDepthFunc(GL_LESS); // 恢复默认
		
		glfwSwapBuffers(pWindow); // 交换前后缓冲区
		glfwPollEvents(); // 轮询 - glfw 与 窗口通信
	}

	// 清理资源
	glfwDestroyWindow(pWindow); // 销毁窗口
	glfwTerminate();			// 终止GLFW
}