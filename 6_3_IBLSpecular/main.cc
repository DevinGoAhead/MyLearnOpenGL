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

	auto SetVertices = [](GLuint& VBO, const std::vector<float>& vertices){
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
	};

	// cube
	GLuint cubeVBO,cubeVAO;
	SetVertices(cubeVBO,cubeVertices);
	auto SetVAOPosNorTex = [](GLuint& VAO){
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);
		
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0); // 顶点
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float))); // 法线
		glEnableVertexAttribArray(1);

		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float))); // 纹理
		glEnableVertexAttribArray(2);

		glBindVertexArray(0); //重置默认绑定
	};
	SetVAOPosNorTex(cubeVAO);

	// quad
	GLuint quadVBO, quadVAO;
	SetVertices(quadVBO,quadVertices);
	auto SetVAOPosTex = [](GLuint& VAO){
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);
		
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0); // 顶点
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float))); // 纹理
		glEnableVertexAttribArray(1);

		glBindVertexArray(0); //重置默认绑定
	};
	SetVAOPosTex(quadVAO);

	// 创建纹理对象
	auto setTexParameter = [](GLuint& tex, GLenum target, GLint wrappingParam, GLint magFilterParam, GLint minFilterParam){
		glGenTextures(1, &tex);
		glBindTexture(target, tex);
		// 设置纹理环绕参数
		if(wrappingParam != -1) {
			glTexParameteri(target, GL_TEXTURE_WRAP_S, wrappingParam);
			glTexParameteri(target, GL_TEXTURE_WRAP_T, wrappingParam);
			if(target == GL_TEXTURE_CUBE_MAP) glTexParameteri(target, GL_TEXTURE_WRAP_R, wrappingParam);

			if(wrappingParam == GL_CLAMP_TO_BORDER) {
				GLfloat borderColor[] ={1.0f, 1.0f, 1.0f, 1.0f};
				glTexParameterfv(target, GL_TEXTURE_BORDER_COLOR, borderColor);
			}
		}
		// 设置纹理映射(过滤)方式
		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, magFilterParam); // 放大
		glTexParameteri(target, GL_TEXTURE_MIN_FILTER, minFilterParam); // 缩小
	};
		
	//sphere 纹理
	std::vector<std::string> texNames{"grass", "gold", "wall", "plastic", "rusted_iron"};
	wxy::Material mGrass, mGold, mWall, mPlastic, mRustedIron;
	std::vector<wxy::Material> materials{mGrass, mGold, mWall, mPlastic, mRustedIron};
	uint i = 0;
	for(const auto& texName : texNames) {
		materials[i].PushBackTexture(wxy::Albedo, "../resources/textures/pbr/" + texName + "/albedo.png", true);
		materials[i].PushBackTexture(wxy::Normal, "../resources/textures/pbr/" + texName + "/normal.png", false);
		materials[i].PushBackTexture(wxy::Metallicity, "../resources/textures/pbr/" + texName + "/metallic.png", false);
		materials[i].PushBackTexture(wxy::Roughness, "../resources/textures/pbr/" + texName + "/roughness.png", false);
		materials[i++].PushBackTexture(wxy::AO, "../resources/textures/pbr/" + texName + "/ao.png", true);
	}

	// env 纹理
	uint envHDRTex;
	setTexParameter(envHDRTex, GL_TEXTURE_2D, GL_CLAMP_TO_EDGE, GL_LINEAR,  GL_LINEAR);
	// 加载纹理
	stbi_set_flip_vertically_on_load(true);
	int imageWidth, imageHeight, nChannels;
	float* pImageData = stbi_loadf("../resources/textures/hdr/newport_loft.hdr", &imageWidth, &imageHeight, &nChannels, 0);
	if(!pImageData) {
		std::cerr << "failed to load Image" << '\n';
		exit(1);
	}
	// 这里不是很明白, 作者在文章中说明了 radiance HDR 的第4个通道是 指数
	// 但这里加载却使用了 GL_RGB, 而不是 GL_RGBA?
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, imageWidth, imageHeight, 0, GL_RGB, GL_FLOAT, pImageData); // 开辟内存, 存储图片
	stbi_image_free(pImageData);

	// envCubeFBO, 将环境贴图从等距柱状投影图采样到 cube map 中
	uint envCubeFBO;
	int ecFBOWidth = 512, ecFBOHeight = ecFBOWidth;
	glGenFramebuffers(1, &envCubeFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, envCubeFBO);
	
	uint envCubeTex;
	//从 envCube mipmap 上采样预计算镜面光颜色
	setTexParameter(envCubeTex, GL_TEXTURE_CUBE_MAP, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR);
	for(int i = 0; i < 6; ++i) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, ecFBOWidth, ecFBOHeight, 0, GL_RGB, GL_FLOAT, NULL);
	}
	// 去掉了RBO, 不需要深度测试

	// 采样环境立方体贴图, 预计算漫反射 irradiance,存储到 irradianceCubeTex
	// 这里 diffuse 和 specular 共用 FBO
	uint IBLCubeFBO;
	glGenFramebuffers(1, &IBLCubeFBO);

	//for diffuse irradiance 
	int dCubeFBOWidth = 32, dCubeFBOHeight = dCubeFBOWidth;
	
	uint dCubeTex;
	setTexParameter(dCubeTex, GL_TEXTURE_CUBE_MAP, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR);
	for(int i = 0; i < 6; ++i) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, dCubeFBOWidth, dCubeFBOHeight, 0, GL_RGB, GL_FLOAT, NULL);
	}
	//待使用时, 颜色缓冲再绑定帧缓冲

	//for specular preFilterColor
	int sColorCubeFBOWidth = 128, sColorCubeFBOHeight = sColorCubeFBOWidth;

	uint sCubeTex;
	setTexParameter(sCubeTex, GL_TEXTURE_CUBE_MAP, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR); //这里将会使用三线性插值
	
	
	/*************** 这是手动创建 mipmap 的方式 ***************/ 
	uint mipMapLevels = 7;
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, mipMapLevels - 1); // 指定最大层级

	for(uint face = 0; face < 6; ++face) {
		for(uint level = 0; level < mipMapLevels; ++level) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, GL_RGB16F,
					sColorCubeFBOWidth >> level, sColorCubeFBOHeight >> level, 0, GL_RGB, GL_FLOAT, NULL);
		}
	}

	// for specular LUT(preBRDF)
	int sLUCubeFBOWidth = 512, sLUCubeFBOHeight = sLUCubeFBOWidth;
	
	uint sLUCubeTex; //2D 纹理, 2个通道
	setTexParameter(sLUCubeTex, GL_TEXTURE_2D, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, sLUCubeFBOWidth, sLUCubeFBOHeight, 0, GL_RG, GL_FLOAT, NULL);
	//待使用时, 颜色缓冲再绑定帧缓冲

	// 为cube 构造投影矩阵, 同时为每个面构造视图矩阵, 摄像机一直在中心, 旋转切换方向
	glm::mat4 ecProjection = glm::perspective(glm::radians(90.f), (float)ecFBOWidth / ecFBOHeight, 0.1f, 10.f); // fov-90 确保能看到所有内容
	std::vector<glm::mat4> ecViews {
		glm::lookAt(glm::vec3(0.f), glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f, -1.f, 0.f)),
		glm::lookAt(glm::vec3(0.f), glm::vec3(-1.f, 0.f, 0.f), glm::vec3(0.f, -1.f, 0.f)),
		glm::lookAt(glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.f, 0.f, 1.f)),
		glm::lookAt(glm::vec3(0.f), glm::vec3(0.f, -1.f, 0.f), glm::vec3(0.f, 0.f, -1.f)),
		glm::lookAt(glm::vec3(0.f), glm::vec3(0.f, 0.f, 1.f), glm::vec3(0.f, -1.f, 0.f)),
		glm::lookAt(glm::vec3(0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec3(0.f, -1.f, 0.f))
	};

	wxy::Sphere sphere;
	wxy::ShaderProgram shaderPrgmPBR("./shader/pbr.vs", "./shader/pbr.fs");
	wxy::ShaderProgram shaderPrgmEqRectToCube("./shader/cube.vs", "./shader/eqRectToCube.fs");
	wxy::ShaderProgram shaderPrgmDiffEMap("./shader/cube.vs", "./shader/diffEMap.fs");
	wxy::ShaderProgram shaderPrgmSpecColorMap("./shader/cube.vs", "./shader/specColorMap.fs");
	wxy::ShaderProgram shaderPrgmSpecBRDFMap("./shader/quad.vs", "./shader/specBRDFMap.fs");
	wxy::ShaderProgram shaderPrgmSkyCubeMap("./shader/skyCube.vs", "./shader/skyCube.fs");

	//lights
	std::vector <glm::vec3> lightPositions {
		{-10.0f,  10.0f, 10.0f},
		{ 10.0f,  10.0f, 10.0f},
		{-10.0f, -10.0f, 10.0f},
		{ 10.0f, -10.0f, 10.0f}
	};
	std::vector <glm::vec3> lightColors {
		{300.0f, 300.0f, 300.0f},
		{300.0f, 300.0f, 300.0f},
		{300.0f, 300.0f, 300.0f},
		{300.0f, 300.0f, 300.0f}
	};
	float spacing = 2.5;
	
	//ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext(NULL);
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(pWindow, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	// 主循环
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS); // 在立方体面之间进行线性插值
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glfwSwapInterval(1); // 前后缓冲区交换间隔
	while(!glfwWindowShouldClose(pWindow)) {
		curTime = glfwGetTime();
		perFrameTime = curTime - lastTime;
		lastTime = curTime;

		glClearColor(0.1, 0.1,0.1, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		//ImGui::ShowDemoWindow();
		ImGui::Begin("Control Pannel");

		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 projection = glm::perspective(glm::radians(camera.GetFov()), (float)wndWidth / wndHeight, 0.1f, 100.f);

		// create cube map texture
		glBindFramebuffer(GL_FRAMEBUFFER, envCubeFBO);
		glViewport(0, 0, ecFBOWidth, ecFBOHeight);

		shaderPrgmEqRectToCube.Use();
		shaderPrgmEqRectToCube.SetUniformv("uProjection", ecProjection);
		
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, envHDRTex);
		shaderPrgmEqRectToCube.SetUniform("uTextureEqRect", 0);
		
		glBindVertexArray(cubeVAO);
		for(int i = 0; i < 6; ++i) {
			shaderPrgmEqRectToCube.SetUniformv("uView", ecViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubeTex, 0);
			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
					std::cout << " envCubeFBO is not complete!" << std::endl;
			}
			glClear(GL_COLOR_BUFFER_BIT);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
		glBindTexture(GL_TEXTURE_CUBE_MAP, envCubeTex);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

		// convolution irradiance
		glBindFramebuffer(GL_FRAMEBUFFER, IBLCubeFBO);
		glViewport(0, 0, dCubeFBOWidth, dCubeFBOHeight);

		shaderPrgmDiffEMap.Use();
		shaderPrgmDiffEMap.SetUniformv("uProjection", ecProjection);
		
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, envCubeTex);
		shaderPrgmDiffEMap.SetUniform("uTextureCube", 0);
		
		glBindVertexArray(cubeVAO);
		for(int i = 0; i < 6; ++i) {
			shaderPrgmDiffEMap.SetUniformv("uView", ecViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, dCubeTex, 0);
			
			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
				std::cout << "IBLCubeFBO for diffuse light is not complete!" << std::endl;
			}
			glClear(GL_COLOR_BUFFER_BIT);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		// convolution env specular color
		shaderPrgmSpecColorMap.Use();
		shaderPrgmSpecColorMap.SetUniformv("uProjection", ecProjection);
		shaderPrgmSpecColorMap.SetUniform("uResolution", ecFBOWidth);
	
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, envCubeTex);
		shaderPrgmSpecColorMap.SetUniform("uTextureCube", 0);
		
		glBindVertexArray(cubeVAO);

		for(uint face = 0; face < 6; ++face) {
			shaderPrgmSpecColorMap.SetUniformv("uView", ecViews[face]);
			for(uint level = 0; level < mipMapLevels; ++level) {
				glViewport(0, 0, sColorCubeFBOWidth >> level, sColorCubeFBOWidth >> level);
				shaderPrgmSpecColorMap.SetUniform("uRoughness", (float)level / (mipMapLevels - 1));
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, sCubeTex, level);
				// 一个新的查错的方式
				GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
				if (status != GL_FRAMEBUFFER_COMPLETE) {
				    std::cout << "Framebuffer error: " << status << std::endl;
				}
				glClear(GL_COLOR_BUFFER_BIT);
				glDrawArrays(GL_TRIANGLES, 0, 36);
			}
		}

		// convolution env specular BRDF
		glViewport(0, 0, sLUCubeFBOWidth, sLUCubeFBOHeight);
		shaderPrgmSpecBRDFMap.Use();
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, sLUCubeTex, 0);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			std::cout << "IBLCubeFBO for specular BRDF is not complete!" << std::endl;
		}
		glClear(GL_COLOR_BUFFER_BIT);
		
		glBindVertexArray(quadVAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		// draw background
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, wndWidth, wndHeight);

		shaderPrgmSkyCubeMap.Use();
		shaderPrgmSkyCubeMap.SetUniformv("uView", view);
		shaderPrgmSkyCubeMap.SetUniformv("uProjection", projection);

		static float mMapSampLevel = 0;
		ImGui::SliderFloat("Level of sampling background", &mMapSampLevel, 0.f, 4.f);
		shaderPrgmSkyCubeMap.SetUniform("uMMapSampLevel", mMapSampLevel);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, envCubeTex);
		//glBindTexture(GL_TEXTURE_CUBE_MAP, sCubeTex);
		shaderPrgmSkyCubeMap.SetUniform("uTextureCube", 0);
		
		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		
		// draw sphere
		shaderPrgmPBR.Use();
		shaderPrgmPBR.SetUniformv("uView", view);
		shaderPrgmPBR.SetUniformv("uProjection", projection);
		shaderPrgmPBR.SetUniformv("uCameraPosition", camera.GetPos());
		shaderPrgmPBR.SetUniformv("uLightColors", 4, lightColors.data());
		shaderPrgmPBR.SetUniformv("uLightPositions", 4, lightPositions.data());

		shaderPrgmPBR.SetUniformv("uF0", glm::vec3(0.04)); //基础反射率
		shaderPrgmPBR.SetUniform("uMaxMipLevel", (int)mipMapLevels - 1);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, dCubeTex);
		shaderPrgmPBR.SetUniform("uTextureDiffE", 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_CUBE_MAP, sCubeTex);
		shaderPrgmPBR.SetUniform("uTextureSpecColor", 1);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, sLUCubeTex);
		shaderPrgmPBR.SetUniform("uTextureSpecBRDF", 2);

		glm::mat4 sphereModel = glm::translate(glm::mat4(1.f), glm::vec3(-2.f * spacing, 0.f, 0.f));
		for(auto& material : materials) {
			sphereModel = glm::translate(sphereModel, glm::vec3(spacing, 0.f, 0.f));
				shaderPrgmPBR.SetUniformv("uModel", sphereModel);
				material.Bind(shaderPrgmPBR, 2);
				sphere.Draw();
		}

		// // debug
		// glBindFramebuffer(GL_FRAMEBUFFER, 0);
		// glViewport(0, 0, wndWidth, wndHeight);
		// glDisable(GL_DEPTH_TEST);
		// glBindTexture(GL_TEXTURE_2D, sLUCubeTex);
		// glBindVertexArray(quadVAO);
		// glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		ImGui::End();
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(pWindow); // 交换前后缓冲区
		glfwPollEvents(); // 轮询 - glfw 与 窗口通信
	}
	// for (int i = 0; i < 5; ++i) {
	// 	SaveTexToPNG(sCubeTex, i, "./renderResult/sColorCubeMipMap/mipLevel_");
	// }
	SaveTexToPNG(sLUCubeTex, "./renderResult/BRDF/");
	// 清理资源
	glfwDestroyWindow(pWindow); // 销毁窗口
	glfwTerminate();			// 终止GLFW
}