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
	
	wxy::ShaderProgram shaderPrgm("./shader/model.vert", "", "./shader/model.frag");
	stbi_set_flip_vertically_on_load(true);
	wxy::Model nanosuit("../resources/objects/nanosuit_reflection/nanosuit.obj");

	wxy::ShaderProgram shaderPrgmNorDspy("./shader/normalDisplay.vert", "./shader/normalDisplay.geom", "./shader/normalDisplay.frag");
	
	glm::vec3 light(1.f, 1.f, 1.f);

	glm::vec3 posLight(glm::vec3(5.f));

	glEnable(GL_DEPTH_TEST);
	// 主循环
	glfwSwapInterval(1); // 前后缓冲区交换间隔
	while(!glfwWindowShouldClose(pWindow))
	{
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);//设置清除颜色缓冲区后要使用的颜色-纯色
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 清楚颜色缓冲区

		curTime = glfwGetTime();
		perFrameTime = curTime - lastTime;
		lastTime = curTime;
		
		//视图和投影变换
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 projection = glm::perspective(glm::radians(camera.GetFov()), (float)wndWidth / (float)wndHeight, 0.1f, 100.f);

		// 模型
		shaderPrgm.Use();
		
		glm::mat4 modelNanosuit =  glm::translate(glm::mat4(1.f), glm::vec3(0.f, -2.f, 0.f));
		modelNanosuit = glm::scale(modelNanosuit, glm::vec3(0.5f));
		modelNanosuit = glm::rotate(modelNanosuit, glm::radians(45.f), glm::vec3(0.f, 1.f, 0.f));

		shaderPrgm.SetUniformv("model_", modelNanosuit);
		shaderPrgm.SetUniformv("view_", view);
		shaderPrgm.SetUniformv("projection_", projection);

		// light
		shaderPrgm.SetUniformv("light_.ambient", glm::vec3(0.2f));
		shaderPrgm.SetUniformv("light_.diffuse", glm::vec3(0.6f));
		shaderPrgm.SetUniformv("light_.specular", glm::vec3(0.8f));
		shaderPrgm.SetUniformv("light_.pos", posLight);
		shaderPrgm.SetUniformv("cameraPos_", camera.GetPos());
		shaderPrgm.SetUniform("uMaterial.shininess", 64);

		nanosuit.Draw(shaderPrgm);

		// 法线可视化
		shaderPrgmNorDspy.Use();
		shaderPrgmNorDspy.SetUniformv("model_", modelNanosuit);
		shaderPrgmNorDspy.SetUniformv("view_", view);
		shaderPrgmNorDspy.SetUniformv("projection_", projection);
		nanosuit.Draw(shaderPrgmNorDspy);
		
		glfwSwapBuffers(pWindow); // 交换前后缓冲区
		glfwPollEvents(); // 轮询 - glfw 与 窗口通信
	}

	// 清理资源
	glfwDestroyWindow(pWindow); // 销毁窗口
	glfwTerminate();			// 终止GLFW
}