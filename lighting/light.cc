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

	// 顶点数据在 global.h 中
	
	GLuint VBO;
	glGenBuffers(1, &VBO); // 创建 vertexBuffer, 绑定到 ID VBO 上

	// 创建纹理对象
	GLuint texture0;
	glGenTextures(1, &texture0); // 创建一个纹理对象, 将其 ID 存储在 texture0 中

	// boxVAO
	GLuint boxVAO;
	glGenVertexArrays(1, &boxVAO); // 创建 vertexArray, 绑定到 ID boxVAO 上
	glBindVertexArray(boxVAO); // 绑定 boxVAO 到 openGL 上下文, 开始记录信息
	glBindBuffer(GL_ARRAY_BUFFER, VBO); // 将 VBO 绑定在 GL_ARRAY_BUFFER 上, 可以将数据传递给顶点着色器
	
	// 在 GPU 中开辟指定类型的缓冲区, 用于存放 vertices indeies
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glActiveTexture(GL_TEXTURE0); // 激活纹理单元0, 并绑定 texture0
	glBindTexture(GL_TEXTURE_2D, texture0);
	// 设置纹理参数
	// 分别设置 ST 方向的环绕方式
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// 设置纹理映射缩放算法
	// 当纹理被放大, 线性插值, 避免像素化
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
	// 当纹理被缩小, 使用 MipMap 算法, 且采用三线性插值的算法
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); 

	// 加载图像
	stbi_set_flip_vertically_on_load(true); // 加载图像时反转 y 轴
	int texWidth, texHeight, nRChannels;
	unsigned char* pImageData = stbi_load("./model/container.jpg", &texWidth, &texHeight, &nRChannels, 0);
	if(!pImageData) 
	{
		std::cerr << "failed to load Image" << '\n';
		exit(1);
	}

	// 生成纹理图像
	// 生成的纹理图像将保存在 glGenTextures 所生成的对象中
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texWidth, texHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, pImageData);
	glGenerateMipmap(GL_TEXTURE_2D);// 自动生成多级渐远纹理
	stbi_image_free(pImageData);// 释放图片资源
	
	// 顶点着色器 location index, 属性元素分量, 类型, 是否标准化, 步距, 相对起点偏移
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), 0); // 顶点坐标
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat))); // 法线坐标(向量)
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat))); // 纹理
	glEnableVertexAttribArray(0); // 启用location 0
	glEnableVertexAttribArray(1); // 启用location 1
	glEnableVertexAttribArray(2); // 启用location 2
	glBindVertexArray(0); // 将 boxVAO 从 OpenGL 当前上下文解绑

	// lightVAO
	// 本例使用的是点光源, 但是使用了 box 作为壳, 使光源能够被观察到
	GLuint lightVAO;
	glGenVertexArrays(1, &lightVAO); // 创建 vertexArray, 绑定到 ID boxVAO 上
	glBindVertexArray(lightVAO); // 绑定 boxVAO 到 openGL 上下文, 开始记录信息
	glBindBuffer(GL_ARRAY_BUFFER, VBO); // 将 VBO 绑定在 GL_ARRAY_BUFFER 上, 可以将数据传递给顶点着色器
	
	//这里不需要再次在 GPU 中开辟内存了
	
	// 顶点着色器 location index, 属性元素分量, 类型, 是否标准化, 步距, 相对起点偏移
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(0); // 启用 location 0
	glBindVertexArray(0); // 将 lightVAO 从 OpenGL 当前上下文解绑
	
	glEnable(GL_BLEND);//启用颜色混合操作功能
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);//设置颜色混合模式
	glEnable(GL_DEPTH_TEST);// 启用深度测试

	ShaderProgram boxShaderProgram("./shader/boxVS.glsl", "./shader/boxFS.glsl");
	ShaderProgram lightShaderProgram("./shader/lightVS.glsl", "./shader/lightFS.glsl");
	glm::vec3 boxColor{0.7f, 0.5f, 0.3f};
	glm::vec3 lightColor{1.f, 1.f, 1.f};
	
	// 主循环
	glfwSwapInterval(1); // 前后缓冲区交换间隔
	while(!glfwWindowShouldClose(pWindow))
	{
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);//设置清除颜色缓冲区后要使用的颜色-纯色
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 清楚颜色缓冲区和深度缓冲区

		curTime = glfwGetTime();
		perFrameTime = curTime - lastTime;
		lastTime = curTime;

		glm::vec3 lightPos{3.f * glm::sin(curTime), 0.f, 3.f * glm::cos(curTime)}; //沿半径为3 的圆周运动
		// 视图变换和投影变换矩阵
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 project = glm::perspective(glm::radians(camera.GetFov()), (float)wndWidth / (float)wndHeight, 0.1f, 100.f); 
		//draw box
		boxShaderProgram.Use();
		glBindVertexArray(boxVAO);
		glm::mat4 boxModel = glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.f, -1.f)); // 3 向 -z 方向移动一下, 确保观察大小合适
		boxModel = glm::rotate(boxModel, 10 * glm::radians(curTime), glm::vec3(0.f, 1.f, 0.f)); // 2. 随时间沿 y 轴旋转
		boxModel = glm::rotate(boxModel, glm::radians(20.f), glm::vec3(1.f, 0.f, 0.f)); //1. 沿 x 轴旋转一个小的角度, 可以看到顶面
		glUniformMatrix4fv(glGetUniformLocation(boxShaderProgram.ID(), "model_"), 1, GL_FALSE, glm::value_ptr(boxModel));
		glUniformMatrix4fv(glGetUniformLocation(boxShaderProgram.ID(), "view_"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(boxShaderProgram.ID(), "project_"), 1, GL_FALSE, glm::value_ptr(project));
		glUniform3fv(glGetUniformLocation(boxShaderProgram.ID(), "boxColor_"), 1, glm::value_ptr(boxColor));
		glUniform3fv(glGetUniformLocation(boxShaderProgram.ID(), "lightColor_"), 1, glm::value_ptr(lightColor));
		glUniform3fv(glGetUniformLocation(boxShaderProgram.ID(), "lightPos_"), 1, glm::value_ptr(lightPos));
		boxShaderProgram.SetUniform("texturer0_", 0);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// draw light
		lightShaderProgram.Use();
		glBindVertexArray(lightVAO);
		
		glm::mat4 lightModel = glm::translate(glm::mat4(1.f), lightPos);// 3. 沿半径为5 的圆周运动
		lightModel = glm::translate(lightModel, glm::vec3(0.f, 0.f, -1.f)); // 2. 移动到和box 在同一个深度上
		lightModel = glm::scale(lightModel, glm::vec3(0.2f, 0.2f, 0.2f));// 1. 适当缩放, 使其能观察到, 不至于太大
		glUniformMatrix4fv(glGetUniformLocation(lightShaderProgram.ID(), "model_"), 1, GL_FALSE, glm::value_ptr(lightModel));
		glUniformMatrix4fv(glGetUniformLocation(lightShaderProgram.ID(), "view_"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(lightShaderProgram.ID(), "project_"), 1, GL_FALSE, glm::value_ptr(project));
		glUniform3fv(glGetUniformLocation(lightShaderProgram.ID(), "lightColor_"), 1, glm::value_ptr(lightColor));
		glUniform3fv(glGetUniformLocation(lightShaderProgram.ID(), "cameraPos_"), 1, glm::value_ptr(camera.GetPos()));
		glDrawArrays(GL_TRIANGLES, 0, 36);
		
		// 渲染
		glfwSwapBuffers(pWindow); // 交换前后缓冲区
		glfwPollEvents(); // 轮询 - glfw 与 窗口通信
	}

	// 清理资源
	glDeleteBuffers(1,&VBO);
	glDeleteVertexArrays(1, &boxVAO);
	//glDeleteVertexArrays(1, &lightVAO);

	glfwDestroyWindow(pWindow); // 销毁窗口
	glfwTerminate();			// 终止GLFW
}