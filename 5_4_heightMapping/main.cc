#include "global.h"

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
	auto SetVertices = [](GLuint& VBO, const std::vector<float>& vertices){
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
	};
	
	auto SetVAOPosTex = [](GLuint& VAO){
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);
		
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0); // 顶点
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float))); // 纹理
		glEnableVertexAttribArray(1);

		glBindVertexArray(0); //重置默认绑定
	};

	// quadrangle
	GLuint quadVBO,quadVAO;
	SetVertices(quadVBO, quadVertices);
	SetVAOPosTex(quadVAO);

	// 设置纹理
	// 创建纹理对象
	auto setTexParameter = [](GLuint& tex, GLenum target, GLint wrappingParam, GLint magFilterParam, GLint minFilterParam){
		glGenTextures(1, &tex);
		//glActiveTexture(GL_TEXTURE0);
		glBindTexture(target, tex);
		// 设置纹理环绕参数
		glTexParameteri(target, GL_TEXTURE_WRAP_S, wrappingParam);
		glTexParameteri(target, GL_TEXTURE_WRAP_T, wrappingParam);
		if(target == GL_TEXTURE_CUBE_MAP) glTexParameteri(target, GL_TEXTURE_WRAP_R, wrappingParam);

		if(wrappingParam == GL_CLAMP_TO_BORDER) {
			GLfloat borderColor[] ={1.0f, 1.0f, 1.0f, 1.0f};
			glTexParameterfv(target, GL_TEXTURE_BORDER_COLOR, borderColor);
		}
		
		// 设置纹理映射(过滤)方式
		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, magFilterParam); // 放大
		glTexParameteri(target, GL_TEXTURE_MIN_FILTER, minFilterParam); // 缩小
	};
	
	auto GenerateTex = [](const char* pathName, bool isGammaCorrect){
		// 加载纹理
		int imageWidth, imageHeight, nChannels;
		unsigned char* pImageData = stbi_load(pathName, &imageWidth, &imageHeight, &nChannels, 0);
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

	GLuint quadTex;
	setTexParameter(quadTex, GL_TEXTURE_2D, GL_REPEAT, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR);
	GenerateTex("../resources/textures/bricks2.jpg", true);

	GLuint quadNorTex; //法线纹理
	setTexParameter(quadNorTex, GL_TEXTURE_2D, GL_REPEAT, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR);
	GenerateTex("../resources/textures/bricks2_normal.jpg",false);

	GLuint quadHeightTex; //法线纹理
	setTexParameter(quadHeightTex, GL_TEXTURE_2D, GL_REPEAT, GL_LINEAR, GL_LINEAR);
	GenerateTex("../resources/textures/bricks2_disp.jpg",false);

	wxy::ShaderProgram shaderPrgmModel("./shader/model.vert", "./shader/model.frag");
	
	// TBN 计算, 我这里仅计算了一组 TBN 基向量, 因为是平面, 一组即可
	glm::vec3 E1 = positions[1] - positions[0];
	glm::vec3 E2 = positions[2] - positions[0];
	
	glm::vec2 dUV1 = uvs[1] - uvs[0];
	glm::vec2 dUV2 = uvs[2] - uvs[0];

	// 写到这里感觉 glm 的矩阵运算好难用阿
	// 尝试了几种办法, 都很别扭
	// 还是决定手动计算

	float dU1 = dUV1.x, dV1 = dUV1.y;
	float dU2 = dUV2.x, dV2 = dUV2.y;
	float f = 1.f / (dU1 * dV2 - dU2 * dV1);

	glm::vec3 tagent = f * glm::vec3(dV2 * E1.x - dV1 * E2.x, dV2 * E1.y - dV1 * E2.y, dV2 * E1.z - dV1 * E2.z);
	//glm::vec3 bitagent = f * glm::vec3(-dU2 * E1.x + dU1 * E2.x, -dU2 * E1.y + dU1 * E2.y, -dU2 * E1.z + dU1 * E2.z);
	//std::cout << normal.x << " " << normal.y<< " " << normal.z;
	tagent = glm::normalize(tagent - normal * (glm::dot(normal, tagent))); // 施密特正交化
	//std::cout << tagent.x << " " << tagent.y<< " " << tagent.z;
	glm::vec3 bitagent = glm::normalize(glm::cross(normal, tagent));
	glm::mat3 matTBN{tagent, bitagent, normal};

	glm::vec3 lightPos{0.5f, 1.0f, 0.3f};

	// 主循环
	glEnable(GL_DEPTH_TEST);
	glfwSwapInterval(1); // 前后缓冲区交换间隔
	
	while(!glfwWindowShouldClose(pWindow)) {
		curTime = glfwGetTime();
		perFrameTime = curTime - lastTime;
		lastTime = curTime;
		
		glClearColor(0.1, 0.1,0.1, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		// 视图和投影变换矩阵
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 projection = glm::perspective(glm::radians(camera.GetFov()), (float)wndWidth / wndHeight, 0.1f, 100.f);

		shaderPrgmModel.Use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, quadTex);
		shaderPrgmModel.SetUniform("uMaterial.textureDiffuse0", 0); // 材质纹理

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, quadNorTex);
		shaderPrgmModel.SetUniform("uMaterial.textureNormal", 1); // 法线纹理
		
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, quadHeightTex);
		shaderPrgmModel.SetUniform("uMaterial.textureHeight", 2); // 深度纹理

		shaderPrgmModel.SetUniformv("uCameraPos", camera.GetPos()); // 相机位置
		shaderPrgmModel.SetUniformv("uLightPos", lightPos); // 光位置
		shaderPrgmModel.SetUniform("uScale", heightScale); // 纹理偏移比例系数

		// matrix
		shaderPrgmModel.SetUniformv("uT", tagent);
		shaderPrgmModel.SetUniformv("uB", bitagent);
		shaderPrgmModel.SetUniformv("uN", normal);
		shaderPrgmModel.SetUniformv("uView", view);
		shaderPrgmModel.SetUniformv("uProjection", projection);
		glm::mat4 model = glm::rotate(glm::mat4(1.f), glm::radians(20.f), glm::vec3(0.f, 1.f, 0.f));
		shaderPrgmModel.SetUniformv("uModel", model);
		
		glBindVertexArray(quadVAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glfwSwapBuffers(pWindow); // 交换前后缓冲区
		glfwPollEvents(); // 轮询 - glfw 与 窗口通信
	}

	// 清理资源
	glfwDestroyWindow(pWindow); // 销毁窗口
	glfwTerminate();			// 终止GLFW
}