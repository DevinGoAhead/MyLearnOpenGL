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

	//std::string pathName = "../resources/objects/vampire/dancing_vampire.dae"; 
	//std::string pathName = "../resources/objects/Catwalk_Walk_Stop_Twist.dae"; 
	std::string pathName = "../resources/objects/warrok/Flying_Knee_Punch_Combo.dae";

	wxy::ShaderProgram animationShaderPrgm("./shader/animation.vert", "./shader/animation.frag");
	wxy::Model actor(std::filesystem::absolute(pathName).string());
	wxy::AnimationsManager animationManager;
	animationManager.LoadAnimations(pathName, actor);
	//animationManager.LoadAnimations("../resources/objects/Great_Sword_Slash.dae", actor);
	animationManager.PrintAnimationsName();
	wxy::Skeleton skeleton(pathName);
	wxy::Animator animator(skeleton);
	//auto animation = animationManager.GetAnimation("Hips");
	animator.Play(animationManager.GetAnimation("mixamorig_Hips"));
	
	// std::cout << '\n';
	// animation->PrintBonesInBones();
	// std::cout << '\n';
	// animation->PrintBonesInInfoMap();
	// std::cout << '\n';
	// skeleton.PrintBonesInHierarchy();

	// 主循环
	glEnable(GL_DEPTH_TEST);
	glfwSwapInterval(1); // 前后缓冲区交换间隔

	while(!glfwWindowShouldClose(pWindow)) {
		curTime = glfwGetTime();
		perFrameTime = curTime - lastTime;
		lastTime = curTime;

		glClearColor(0.1, 0.1,0.1, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 projection = glm::perspective(glm::radians(camera.GetFov()), (float)wndWidth / wndHeight, 0.1f, 100.f);

		animationShaderPrgm.Use();
		animationShaderPrgm.SetUniformv("uView", view);
		animationShaderPrgm.SetUniformv("uProjection", projection);

		animator.UpdateAnimation(perFrameTime);
		auto finalTransforms = animator.GetFinalTransforms();
		animationShaderPrgm.SetUniformv("uFinalTransforms", wxy::MaxBones, animator.GetFinalTransforms().data());
		
		glm::mat4 model = glm::scale(glm::mat4(1.f), glm::vec3(0.5f));
		animationShaderPrgm.SetUniformv("uModel", model);
		actor.Draw(animationShaderPrgm);

		glfwSwapBuffers(pWindow); // 交换前后缓冲区
		glfwPollEvents(); // 轮询 - glfw 与 窗口通信
	}
	// 清理资源
	glfwDestroyWindow(pWindow); // 销毁窗口
	glfwTerminate();			// 终止GLFW
}