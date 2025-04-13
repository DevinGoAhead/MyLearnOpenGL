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

	wxy::ShaderProgram animationShaderPrgm("./shader/animation.vert", "./shader/animation.frag");

	std::string modelPathName = "../resources/objects/warrok/Flying_Knee_Punch_Combo.dae";

	std::vector<std::string> animationPathNames;
	std::string animationPrimaryPath = "../resources/objects/warrok/";
	animationPathNames.emplace_back(animationPrimaryPath + "Flying_Knee_Punch_Combo.dae");
	animationPathNames.emplace_back(animationPrimaryPath + "Swinging.dae");
	animationPathNames.emplace_back(animationPrimaryPath + "Strut_Walking.dae");
	animationPathNames.emplace_back(animationPrimaryPath + "Fall_Flat.dae");
	animationPathNames.emplace_back(animationPrimaryPath + "Flair.dae");
	animationPathNames.emplace_back(animationPrimaryPath + "Running_Arc.dae");

	wxy::Model actor(std::filesystem::absolute(modelPathName).string());
	
	std::vector<std::string> animationNames;
	std::vector<const char*> animationNamesCStr;
	uint sizeAnimaPathNames = animationPathNames.size();
	animationNames.reserve(sizeAnimaPathNames);
	animationNamesCStr.reserve(sizeAnimaPathNames);
	wxy::AnimationsManager animationManager;

	for(const auto& animationPathName : animationPathNames) {
			auto animationName = std::filesystem::path(animationPathName).stem().string();
			animationNames.push_back(animationName);
			animationNamesCStr.push_back(animationNames.back().c_str());
			animationManager.LoadAnimations(animationPathName, animationName, actor);
	}
	animationManager.PrintAnimationsName();

	wxy::Skeleton skeleton(modelPathName);
	wxy::Animator animator(skeleton);

	//ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext(NULL);
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(pWindow, true);
	ImGui_ImplOpenGL3_Init("#version 330");
	ImGuiIO& io = ImGui::GetIO();
	io.FontGlobalScale += 0.8f;

	bool isFirstFrame = true; // 标记首次运行
	// 主循环
	glEnable(GL_DEPTH_TEST);
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

		ImVec2 sizeControPannel(600, 300);
		//ImVec2 posControPannel((wndWidth - sizeControPannel.x)  * 0.5f, 2.f); // 居中, 距离顶部20 像素

		ImGui::SetNextWindowSize(sizeControPannel);
		//ImGui::SetNextWindowPos(posControPannel);
		
		ImGui::Begin("Control Pannel");

		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 projection = glm::perspective(glm::radians(camera.GetFov()), (float)wndWidth / wndHeight, 0.1f, 100.f);

		animationShaderPrgm.Use();
		animationShaderPrgm.SetUniformv("uView", view);
		animationShaderPrgm.SetUniformv("uProjection", projection);
		// 下面
		if (isFirstFrame) {
		    // 程序启动时自动播放第一个动画
		    if (!animationNames.empty()) {
		        animator.Play(animationManager.GetAnimation(animationNames[0]));
		    }
		    isFirstFrame = false;
		}
		// 必须要渲染出第 1 帧,才会触发 ImGui::Combo 并返回true
		// 但是又必须要先进入if, 调用 Play 才能渲染出第一帧, 由此设置了上面的 if (isFirstFrame) 
		static int curAnimationIndex = 0;
		if(ImGui::Combo("Select Animation", &curAnimationIndex, animationNamesCStr.data(), animationNames.size())) {
			animator.Play(animationManager.GetAnimation(animationNames[curAnimationIndex]));
		}

		static wxy::PlayMode curPlayMode = wxy::Once;
		ImGui::BeginGroup();
		ImGui::RadioButton("Loop", (int*)&curPlayMode, wxy::Loop); ImGui::SameLine();
		ImGui::RadioButton("Once", (int*)&curPlayMode, wxy::Once);ImGui::SameLine();
		ImGui::EndGroup();
		animator.UpdateAnimation(perFrameTime, curPlayMode);
		
		auto finalTransforms = animator.GetFinalTransforms();
		animationShaderPrgm.SetUniformv("uFinalTransforms", wxy::MaxBones, animator.GetFinalTransforms().data());
		
		glm::mat4 model = glm::scale(glm::mat4(1.f), glm::vec3(0.5f));
		animationShaderPrgm.SetUniformv("uModel", model);
		actor.Draw(animationShaderPrgm);

		ImGui::End();
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(pWindow); // 交换前后缓冲区
		glfwPollEvents(); // 轮询 - glfw 与 窗口通信
	}
	// 清理资源
	glfwDestroyWindow(pWindow); // 销毁窗口
	glfwTerminate();			// 终止GLFW
}