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
	GLFWwindow* pWindow = glfwCreateWindow(800, 800, "lighting", NULL, NULL);
	glfwMakeContextCurrent(pWindow); // 设置 pWindow 窗口为当前上下文

	if(!gladLoadGLLoader(GLADloadproc(glfwGetProcAddress))) {std::cerr << "Initilize GLAD error" << std::endl;}// 初始化 GLAD

	glfwSetFramebufferSizeCallback(pWindow,viewport_size_callback);// 注册视口尺寸自动调整回调函数
	glfwMaximizeWindow(pWindow); // 最大化窗口
	
	while (wndWidth == 0 || wndHeight == 0) {
        glfwPollEvents();  // Make sure GLFW is processing events
    }
	
	
	glfwSetKeyCallback(pWindow, key_callback);// 注册键盘动作处理回调函数
	glfwSetCursorPosCallback(pWindow, cursor_callback);// 注册光标捕捉函数
	glfwSetScrollCallback(pWindow, scroll_callback);// 注册滚轮捕捉函数
	
	wxy::Model planet("../resources/objects/planet/planet.obj");// 行星
	wxy::Model asteroid("../resources/objects/rock/rock.obj"); // 小行星带
	
	wxy::ShaderProgram SdPmPlanet("./shader/planet.vert", "./shader/planet.frag");
	wxy::ShaderProgram SdPmAsteroid("./shader/asteroid.vert", "./shader/planet.frag");
	GetError();

	// 小行星参数
	uint amount = 1000; // 数量

	float radius = 10; // 分布半径
	std::random_device rd; // 随机数生成器, 用于生成一个随机数种子
	std::mt19937 gen(rd()); // 这是实际生成随机数的引擎
	std::vector<glm::mat4> modelAsteroids;
	modelAsteroids.reserve(amount);
	// 小行星带分布在半径为radius 的圆上, 但是位置会在圆周的的周围做微小的随即波动
	for(uint i = 0; i < amount; ++i) {
		float angle = (float)i / amount * 360; // 确保小行星均匀分布
		// 位置
		std::uniform_real_distribution<float> distrPos(-2.5f, 2.5f); // 位置波动范围
		float x = sin(angle) * radius + distrPos(gen);
		float z = cos(angle) * radius + distrPos(gen);
		float y = distrPos(gen) * 0.5f;
		glm::mat modelAsteroid = glm::translate(glm::mat4(1.f), glm::vec3(x, y, z));

		// 缩放
		std::uniform_real_distribution<float> distrScale(0.05f, 0.25f); // 缩放波动范围
		modelAsteroid = glm::scale(modelAsteroid, glm::vec3(distrScale(gen)));

		// 旋转
		std::uniform_real_distribution<float> distrRotate(0.f, 360.f); // 旋转波动范围
		modelAsteroid = glm::rotate(modelAsteroid, distrRotate(gen), glm::vec3(0.2f, 0.5f, 0.8f));
		modelAsteroids.push_back(modelAsteroid);
	}

	glm::vec3 lightPos = glm::vec3(20.f, 20.f, 0.f);
	// 主循环
	glfwSwapInterval(1); // 前后缓冲区交换间隔
	glEnable(GL_DEPTH_TEST);
	while(!glfwWindowShouldClose(pWindow))
	{
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);//设置清除颜色缓冲区后要使用的颜色-纯色
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 清除缓冲区

		curTime = glfwGetTime();
		perFrameTime = curTime - lastTime;
		lastTime = curTime;

		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 projection = glm::perspective(glm::radians(camera.GetFov()), (float)wndWidth / (float)wndHeight, 0.1f, 100.f);
		// 绘制行星

		SdPmPlanet.Use();
		// 模型
		SdPmPlanet.SetUniformv("uView", view);
		//GetError();
		SdPmPlanet.SetUniformv("uProjection", projection);
		glm::mat4 modelPlanet = glm::rotate(glm::mat4(1.f), (float)glm::radians(curTime) * 5.f, glm::vec3(0.f, 1.f, 0.f));
		modelPlanet = glm::scale(modelPlanet, glm::vec3(1.5f));

		SdPmPlanet.SetUniformv("uModel", modelPlanet);
		
		SdPmPlanet.SetUniformv("uCameraPos", camera.GetPos()); // 相机
		// 灯光
		SdPmPlanet.SetUniformv("uLight.ambient", glm::vec3(0.08f));
		SdPmPlanet.SetUniformv("uLight.diffuse", glm::vec3(0.6f));
		SdPmPlanet.SetUniformv("uLight.specular", glm::vec3(0.1f));
		//GetError();
		SdPmPlanet.SetUniformv("uLight.pos", lightPos);

		SdPmPlanet.SetUniform("uMaterial.shininess", 32);// 材质

		planet.Draw(SdPmPlanet);
		//GetError();

		// 绘制小行星带

		SdPmAsteroid.Use();
		// 模型
		SdPmAsteroid.SetUniformv("uView", view);
		SdPmAsteroid.SetUniformv("uProjection", projection);

		SdPmAsteroid.SetUniformv("uCameraPos", camera.GetPos()); // 相机
		// 灯光
		SdPmAsteroid.SetUniformv("uLight.ambient", glm::vec3(0.1f));
		SdPmAsteroid.SetUniformv("uLight.diffuse", glm::vec3(0.6f));
		SdPmAsteroid.SetUniformv("uLight.specular", glm::vec3(0.5f));
		SdPmAsteroid.SetUniformv("uLight.pos", lightPos);

		SdPmAsteroid.SetUniform("uMaterial.shininess", 64);// 材质
		//GetError();

		// 小行星带分布在半径为radius 的圆上, 但是位置会在圆周的的周围做微小的随即波动
		for(uint i = 0; i < amount; ++i) {
			SdPmAsteroid.SetUniformv("uModel", modelAsteroids[i]);
			asteroid.Draw(SdPmAsteroid);
		}
		//glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 100); // 绘制 100 个实例
		
		glfwSwapBuffers(pWindow); // 交换前后缓冲区
		glfwPollEvents(); // 轮询 - glfw 与 窗口通信
	}

	// 清理资源
	glfwDestroyWindow(pWindow); // 销毁窗口
	glfwTerminate();			// 终止GLFW
}