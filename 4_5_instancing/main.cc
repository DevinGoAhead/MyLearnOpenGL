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

	wxy::ShaderProgram shaderPrgm("./shader/model.vert", "", "./shader/model.frag");
	shaderPrgm.Use();
	
	// 顶点数据在 global.h 中
	GLuint quadVBO, quadVAO;
	std::function<void(void)> QuadVertAttrPtr = [](){
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0); // 顶点坐标
			glEnableVertexAttribArray(0); // 启用location 0
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float))); // 颜色
			glEnableVertexAttribArray(1); // 启用location 1

			glBindVertexArray(0); // 将 VAO 从 OpenGL 当前上下文解绑
	};
	SetVertices(quadVBO, quadVAO, quadVertices, QuadVertAttrPtr);

	std::vector<glm::vec2> offsetVertices(100);
	for(uint y = 0; y < 10; ++y) {
		for(uint x = 0; x < 10; ++x) {
			offsetVertices[10 * y + x] = {0.2f * x - 0.9f, 0.2f * y - 0.9f};
			//shaderPrgm.SetUniformv("uOffsets[" + std::to_string(index) + "]", offsetVertices[index]); //添加到 uniform 变量
		}
	}
	shaderPrgm.SetUniformv("uOffsets", 100, offsetVertices.data()); //批量设置
	GLenum err = glGetError();
	if (err != GL_NO_ERROR) {
	    std::cerr << "OpenGL Error: " << err << std::endl;
	}

	// 主循环
	glfwSwapInterval(1); // 前后缓冲区交换间隔
	while(!glfwWindowShouldClose(pWindow))
	{
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);//设置清除颜色缓冲区后要使用的颜色-纯色
		glClear(GL_COLOR_BUFFER_BIT); // 清除缓冲区

		curTime = glfwGetTime();
		perFrameTime = curTime - lastTime;
		lastTime = curTime;

		shaderPrgm.Use();
		glBindVertexArray(quadVAO);
		glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 100); // 绘制 100 个实例
		
		glfwSwapBuffers(pWindow); // 交换前后缓冲区
		glfwPollEvents(); // 轮询 - glfw 与 窗口通信
	}

	// 清理资源
	glfwDestroyWindow(pWindow); // 销毁窗口
	glfwTerminate();			// 终止GLFW
}