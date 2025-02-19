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
	// boxVAO
	GLuint boxVAO;
	glGenVertexArrays(1, &boxVAO); // 创建 vertexArray, 绑定到 ID boxVAO 上
	glBindVertexArray(boxVAO); // 绑定 boxVAO 到 openGL 上下文, 开始记录信息
	glBindBuffer(GL_ARRAY_BUFFER, VBO); // 将 VBO 绑定在 GL_ARRAY_BUFFER 上, 可以将数据传递给顶点着色器
	
	// 在 GPU 中开辟指定类型的缓冲区, 用于存放 vertices indeies
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	
	// 顶点着色器 location index, 属性元素分量, 类型, 是否标准化, 步距, 相对起点偏移
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GL_FLOAT), 0);
	glEnableVertexAttribArray(0); // 启用location 0
	glBindVertexArray(0); // 将 boxVAO 从 OpenGL 当前上下文解绑

	//lightVAO, 但是我觉得在当前练习中, 使用不同的 VAO 没有任何意义
	GLuint lightVAO;
	glGenVertexArrays(1, &lightVAO); // 创建 vertexArray, 绑定到 ID boxVAO 上
	glBindVertexArray(lightVAO); // 绑定 boxVAO 到 openGL 上下文, 开始记录信息
	glBindBuffer(GL_ARRAY_BUFFER, VBO); // 将 VBO 绑定在 GL_ARRAY_BUFFER 上, 可以将数据传递给顶点着色器
	
	//这里不需要再次在 GPU 中开辟内存了
	
	// 顶点着色器 location index, 属性元素分量, 类型, 是否标准化, 步距, 相对起点偏移
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GL_FLOAT), 0);
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
		glClearColor(0.1f, 0.3f, 0.5f, 1.0f);//设置清除颜色缓冲区后要使用的颜色-纯色
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 清楚颜色缓冲区和深度缓冲区

		curTime = glfwGetTime();
		perFrameTime = curTime - lastTime;
		lastTime = curTime;
		// 视图变换和投影变换矩阵
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 project = glm::perspective(glm::radians(camera.GetFov()), (float)wndWidth / (float)wndHeight, 0.1f, 100.f); 
		//draw box
		boxShaderProgram.Use();
		glBindVertexArray(boxVAO);
		glm::mat4 boxModel = glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.f, -5.f));
		boxModel = glm::rotate(boxModel, glm::radians(45.f), glm::vec3(0.f, 0.f, 1.f));
		glUniformMatrix4fv(glGetUniformLocation(boxShaderProgram.ID(), "model_"), 1, GL_FALSE, glm::value_ptr(boxModel));
		glUniformMatrix4fv(glGetUniformLocation(boxShaderProgram.ID(), "view_"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(boxShaderProgram.ID(), "project_"), 1, GL_FALSE, glm::value_ptr(project));
		glUniform3fv(glGetUniformLocation(boxShaderProgram.ID(), "boxColor_"), 1, glm::value_ptr(boxColor));
		glUniform3fv(glGetUniformLocation(boxShaderProgram.ID(), "lightColor_"), 1, glm::value_ptr(lightColor));
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// draw light
		lightShaderProgram.Use();
		glBindVertexArray(lightVAO);
		glm::mat4 lightModel = glm::translate(glm::mat4(1.f), glm::vec3(15.f, 4.f, -40.f)); // 将光源向远处, 右上角移动
		glUniformMatrix4fv(glGetUniformLocation(lightShaderProgram.ID(), "model_"), 1, GL_FALSE, glm::value_ptr(lightModel));
		glUniformMatrix4fv(glGetUniformLocation(lightShaderProgram.ID(), "view_"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(lightShaderProgram.ID(), "project_"), 1, GL_FALSE, glm::value_ptr(project));
		glUniform3fv(glGetUniformLocation(lightShaderProgram.ID(), "lightColor_"), 1, glm::value_ptr(lightColor));
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