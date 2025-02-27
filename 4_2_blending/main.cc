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

	glfwGetFramebufferSize(pWindow, &wndWidth, &wndHeight); // 获取缓冲区尺寸
	glViewport(0, 0, wndWidth, wndHeight); // 设定视口尺寸
	
	glfwSetFramebufferSizeCallback(pWindow,viewport_size_callback);// 注册视口尺寸自动调整回调函数
	glfwSetKeyCallback(pWindow, key_callback);// 注册键盘动作处理回调函数
	glfwSetCursorPosCallback(pWindow, cursor_callback);// 注册光标捕捉函数
	glfwSetScrollCallback(pWindow, scroll_callback);// 注册滚轮捕捉函数
	
	// 顶点
	// VBO VAO 只能是引用传递或指针传递
	auto SetVertices = [](GLuint& VBO, GLuint& VAO, const std::vector<float>& Vertices){
		glGenBuffers(1, &VBO); // 创建 Buffer 对象, 绑定到 ID VBO 上
		glBindBuffer(GL_ARRAY_BUFFER, VBO); // 将 VBO 绑定在 GL_ARRAY_BUFFER 上

		// 在 GPU 中开辟指定类型的缓冲区, 用于存放 vertices indeies
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * Vertices.size(), &Vertices[0], GL_STATIC_DRAW);

		glGenVertexArrays(1, &VAO); // 创建 vertexArray, 绑定到 ID VAO 上
		glBindVertexArray(VAO); // 绑定 VAO 到 openGL 上下文, 开始记录信息

		// 顶点着色器 location index, 属性元素分量, 类型, 是否标准化, 步距, 相对起点偏移
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0); // 顶点坐标
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float))); // 纹理
		glEnableVertexAttribArray(0); // 启用location 0
		glEnableVertexAttribArray(1); // 启用location 1

		glBindVertexArray(0); // 将 VAO 从 OpenGL 当前上下文解绑
	};

	// 顶点数据在 global.h 中
	GLuint cubeVBO, cubeVAO;
	SetVertices(cubeVBO, cubeVAO, cubeVertices);

	GLuint planeVBO, planeVAO;
	SetVertices(planeVBO, planeVAO, planeVertices);

	GLuint trasptVBO, trasptVAO;
	SetVertices(trasptVBO, trasptVAO,transparentVertices);

	//纹理
	// 纹理参数设置
	auto TexParameteri = [](){
		// 2D 纹理, ST 坐标, 超出边界重复
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		// 纹理映射缩放算法
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);//纹理若被放大, 则线性插值
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);//纹理若被缩小, 则三线性插值
	};

	// 加载并生成纹理图像
	auto GenerateTexImg = [](const char* pathName){
		int texWidth, texHeight, nRChannels;
		unsigned char* pImageData = stbi_load(pathName, &texWidth, &texHeight, &nRChannels, 0);
		if(!pImageData) 
		{
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
	TexParameteri(); // 设置纹理参数
	GenerateTexImg("../resources/textures/metal.png");// 加载并生成纹理对象

	GLuint texture1;
	glGenTextures(1, &texture1); // 将纹理对象绑定到的 ID 绑定到texture1
	// 激活纹理单元1, 并绑定 texture1
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture1);
	TexParameteri(); // 设置纹理参数
	GenerateTexImg("../resources/textures/marble.jpg");// 加载并生成纹理对象

	GLuint texture2;
	glGenTextures(1, &texture2); // 将纹理对象绑定到的 ID 绑定到texture1
	// 激活纹理单元2, 并绑定 texture2
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, texture2);
	TexParameteri(); // 设置纹理参数
	//stbi_set_flip_vertically_on_load(true); // 纹理坐标已经设置反转了，这里不需要再次设置了
	GenerateTexImg("../resources/textures/grass.png");// 加载并生成纹理对象
	
	wxy::ShaderProgram shaderProgram("./shader/blend.vert", "./shader/blend.frag");

	glEnable(GL_DEPTH_TEST);// 启用深度测试
	glDepthFunc(GL_LESS); // 禁用深度测试再启用后, 不需要重新配置深度函数

	// 主循环
	glfwSwapInterval(1); // 前后缓冲区交换间隔
	while(!glfwWindowShouldClose(pWindow))
	{
		
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);//设置清除颜色缓冲区后要使用的颜色-纯色
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // 清楚颜色缓冲区, 深度缓冲区, 模板缓冲

		curTime = glfwGetTime();
		perFrameTime = curTime - lastTime;
		lastTime = curTime;
		
		shaderProgram.Use(); // 绘制初始箱子和地板,草

		// 视图变换矩阵和投影变换矩阵是所有对象共用
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 project = glm::perspective(glm::radians(camera.GetFov()), (float)wndWidth / (float)wndHeight, 0.1f, 100.f); 
		shaderProgram.SetUniformv("view_", 1, view);
		shaderProgram.SetUniformv("project_", 1, project);
		
		// draw plane
		// texture
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture1);
		shaderProgram.SetUniform("texturer0_",1);

		glBindVertexArray(planeVAO);
		shaderProgram.SetUniformv("model_", 1, glm::mat4(1.f)); // 模型矩阵
		glDrawArrays(GL_TRIANGLES, 0, 6);
		// 此时模板缓冲值均为0
		
		// draw cube
		// 纹理
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture0);
		shaderProgram.SetUniform("texturer0_",0);

		glBindVertexArray(cubeVAO);
		for(const auto& position : cubePositions) {
			glm::mat4 cubeModel = glm::translate(glm::mat4(1.f), position);
			shaderProgram.SetUniformv("model_", 1, cubeModel);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		// draw vegetation
		// 纹理
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, texture2);
		shaderProgram.SetUniform("texturer0_",2);

		glBindVertexArray(trasptVAO);
		for(const auto& position : vegetationPositions) {
			glm::mat4 vegetationModel = glm::translate(glm::mat4(1.f), position);
			shaderProgram.SetUniformv("model_", 1, vegetationModel);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
		
		glfwSwapBuffers(pWindow); // 交换前后缓冲区
		glfwPollEvents(); // 轮询 - glfw 与 窗口通信
	}

	// 清理资源
	glfwDestroyWindow(pWindow); // 销毁窗口
	glfwTerminate();			// 终止GLFW
}