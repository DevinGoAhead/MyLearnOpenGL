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

	//纹理
	GLuint texture0;
	glGenTextures(1, &texture0); // 将纹理对象绑定到的 ID 绑定到texture0

	GLuint texture1;
	glGenTextures(1, &texture1); // 将纹理对象绑定到的 ID 绑定到texture1

	// boxVAO
	GLuint boxVAO;
	glGenVertexArrays(1, &boxVAO); // 创建 vertexArray, 绑定到 ID boxVAO 上
	glBindVertexArray(boxVAO); // 绑定 boxVAO 到 openGL 上下文, 开始记录信息
	glBindBuffer(GL_ARRAY_BUFFER, VBO); // 将 VBO 绑定在 GL_ARRAY_BUFFER 上, 可以将数据传递给顶点着色器
	
	// 在 GPU 中开辟指定类型的缓冲区, 用于存放 vertices indeies
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	
	// 纹理参数设置
	auto TexParameteri = [](){
		// 2D 纹理, ST 坐标, 超出边界重复
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

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
			std::cerr << "failed to load Image" << '\n';
			exit(1);
		}
		// 本例均为GL_RGBA
		// 生成的纹理图像将保存在 glGenTextures 所生成的对象中
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texWidth, texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, pImageData);
		glGenerateMipmap(GL_TEXTURE_2D);// 自动生成多级渐远纹理
		stbi_image_free(pImageData);// 释放图片资源
	};

	// 激活纹理单元0, 并绑定 texture0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture0);
	TexParameteri(); // 设置纹理参数
	GenerateTexImg("./model/container.png");// 加载并生成纹理对象
	// 激活纹理单元1, 并绑定 texture1
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture1);
	TexParameteri(); // 设置纹理参数
	GenerateTexImg("./model/container2_specular.png");// 加载并生成纹理对象

	// 顶点着色器 location index, 属性元素分量, 类型, 是否标准化, 步距, 相对起点偏移
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), 0); // 顶点坐标
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat))); // 法线坐标(向量)
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat))); // 纹理
	glEnableVertexAttribArray(0); // 启用location 0
	glEnableVertexAttribArray(1); // 启用location 1
	glEnableVertexAttribArray(2); // 启用location 1

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
	glm::vec3 lightColor{1.f, 1.f, 1.f};
	glm::vec3 lightPos{1.2f, 1.0f, 2.0f};

	// 主循环
	glfwSwapInterval(1); // 前后缓冲区交换间隔
	while(!glfwWindowShouldClose(pWindow))
	{
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);//设置清除颜色缓冲区后要使用的颜色-纯色
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 清楚颜色缓冲区和深度缓冲区

		curTime = glfwGetTime();
		perFrameTime = curTime - lastTime;
		lastTime = curTime;
		// //沿半径为 的圆周运动
		//  lightPos.x = 1.0f + sin(curTime) * 2.0f;
        //  lightPos.y = sin(curTime / 2.0f) * 1.0f;
		// 视图变换和投影变换矩阵
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 project = glm::perspective(glm::radians(camera.GetFov()), (float)wndWidth / (float)wndHeight, 0.1f, 100.f); 
		//draw box
		boxShaderProgram.Use();
		glBindVertexArray(boxVAO);

		//light
		//float c_ = glm::sin(curTime);
		//lightColor.x = 0.9f * c_, lightColor.y = 0.8f * glm::cos(curTime), lightColor.z = 0.2 * c_;
		//lightColor.x = 2.f * c_, lightColor.y = 0.7f * c_, lightColor.z = 1.3f * c_;
		glUniform3fv(glGetUniformLocation(boxShaderProgram.ID(), "light_.ambientColor"), 1, glm::value_ptr(glm::vec3(0.2f) * lightColor));
		glUniform3fv(glGetUniformLocation(boxShaderProgram.ID(), "light_.diffuseColor"), 1, glm::value_ptr(glm::vec3(0.8f) * lightColor));
		glUniform3fv(glGetUniformLocation(boxShaderProgram.ID(), "light_.specularColor"), 1, glm::value_ptr(glm::vec3(1.f)* lightColor));
		// 这里想象一个人拿着手电筒, eye 和 light 的坐标相同
		glUniform3fv(glGetUniformLocation(boxShaderProgram.ID(), "light_.pos"), 1, glm::value_ptr(camera.GetPos()));
		glUniform3fv(glGetUniformLocation(boxShaderProgram.ID(), "light_.direc"), 1, glm::value_ptr(glm::normalize(camera.GetFront())));
		
		// 聚光
		// 但仍要考虑衰减
		boxShaderProgram.SetUniform("light_.constant", 1.f); // "光强" 衰减常数项
		boxShaderProgram.SetUniform("light_.linear", 0.07f); // "光强" 衰减一次项(线性衰减)
		boxShaderProgram.SetUniform("light_.quadratic", 0.017f); // "光强" 衰减二次项
		// 直接传递余弦值, 避免在 GPU 中计算
		boxShaderProgram.SetUniform("light_.phi", glm::cos(glm::radians(12.5f))); // 内切光角的余弦
		boxShaderProgram.SetUniform("light_.gamma", glm::cos(glm::radians(17.5f))); // 外切光角的余弦

		//material
		boxShaderProgram.SetUniform("material_.diffuseTexer",0);
		boxShaderProgram.SetUniform("material_.specularTexer",1);
		boxShaderProgram.SetUniform("material_.shiness",64);

		glUniformMatrix4fv(glGetUniformLocation(boxShaderProgram.ID(), "view_"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(boxShaderProgram.ID(), "project_"), 1, GL_FALSE, glm::value_ptr(project));
		glUniform3fv(glGetUniformLocation(boxShaderProgram.ID(), "cameraPos_"), 1, glm::value_ptr(camera.GetPos()));

		//draw 10 boxes
		for(int i = 0; i < 10; ++i)
		{
			glm::mat4 boxModel = glm::translate(glm::mat4(1.f), cubePositions[i]); // 平移到不同的位置
			float angle = i * 20.f;
			boxModel = glm::rotate(boxModel, glm::radians(angle), glm::vec3(0.3f, 0.5f, 0.7f));
			glUniformMatrix4fv(glGetUniformLocation(boxShaderProgram.ID(), "model_"), 1, GL_FALSE, glm::value_ptr(boxModel));
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		// draw ligh
		// lightShaderProgram.Use();
		// glBindVertexArray(lightVAO);
		
		// glm::mat4 lightModel = glm::translate(glm::mat4(1.f), lightPos);// 3. 沿半径为 的圆周运动
		// lightModel = glm::translate(lightModel, glm::vec3(0.f, 0.f, -0.f)); // 2. 移动到和box 在同一个深度上
		// lightModel = glm::scale(lightModel, glm::vec3(0.2f, 0.2f, 0.2f));// 1. 适当缩放, 使其能观察到, 不至于太大
		// glUniformMatrix4fv(glGetUniformLocation(lightShaderProgram.ID(), "model_"), 1, GL_FALSE, glm::value_ptr(lightModel));
		// glUniformMatrix4fv(glGetUniformLocation(lightShaderProgram.ID(), "view_"), 1, GL_FALSE, glm::value_ptr(view));
		// glUniformMatrix4fv(glGetUniformLocation(lightShaderProgram.ID(), "project_"), 1, GL_FALSE, glm::value_ptr(project));
		// glUniform3fv(glGetUniformLocation(lightShaderProgram.ID(), "lightColor_"), 1, glm::value_ptr(lightColor));
		// glUniform3fv(glGetUniformLocation(lightShaderProgram.ID(), "cameraPos_"), 1, glm::value_ptr(camera.GetPos()));
		// glDrawArrays(GL_TRIANGLES, 0, 36);
		
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