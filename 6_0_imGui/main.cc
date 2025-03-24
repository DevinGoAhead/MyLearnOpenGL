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
	
	IMGUI_CHECKVERSION(); //检查ImGui 的头文件版本和编译库版本是否一致, 须在创建上下文之前调用
	ImGui::CreateContext(NULL); // 创建ImGui 的全局上下文, 存储所有状态, NULL表示使用默认配置
	//ImGuiIO& io = ImGui::GetIO(); 

	ImGui::StyleColorsDark(); // 使用默认主题
	ImGui_ImplGlfw_InitForOpenGL(pWindow, true); // 初始化ImGui 的GLFW后端, 用于处理窗口, 输入事件(鼠标, 键盘)
	ImGui_ImplOpenGL3_Init("#version 330"); // 初始化 ImGui 的OpenGL3 后端, 用于处理渲染ImGui的UI

	std::string str = "Initial text"; // Text
	char bufInputTex[64]; // input text
	const char* items[] = {"11", "22", "33", "44", "55", "66", "77"};
	float color[4];
	// 主循环
	glfwSwapInterval(1); // 前后缓冲区交换间隔
	while(!glfwWindowShouldClose(pWindow)) {
		//glClearColor(0.1f, 0.1f, 0.1f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_NewFrame(); //通知 OpenGL后端, 开始新的一帧, 准备处理 UI 渲染数据
		ImGui_ImplGlfw_NewFrame(); // 通知 GLFW 后端, 处理输入事件(鼠标, 键盘等)
		ImGui::NewFrame(); // 开始一个新的 ImGui 帧, 须在所有 UI 控件定义之前调用

		ImGui::ShowDemoWindow(); // 显示ImGui内置演示窗口
		
		ImGui::Begin("MyImGuiWindow");
		
		ImGui::Text(str.c_str()); // 显示静态字体
		if(ImGui::Button("MyButton_1")) { // 单击返回 true
			str = "You have Clik MyButton_1";
		}

		ImGui::InputText("MyTextBox_1", bufInputTex, 64); // 输入框, 将文字写入到 bufInputTex
		ImGui::Text(bufInputTex); // 显示静态字体

		static int number = 4;
		ImGui::Text("select: %d", number); // 显示静态字体
		if(ImGui::BeginListBox("MyListBox_1")) {
			for(int i = 0; i < IM_ARRAYSIZE(items); ++i) { // 每一帧都要遍历一次 list
				const bool isSelect = (number == i);
				// 先检测鼠标是否悬停, 并显示悬停的背景色
				// 进一步检测是否单击, 单击返回true, 也就是说只有单击彩绘执行 number = i
				if(ImGui::Selectable(items[i], isSelect)) {
					number = i;
				}
				if(isSelect) {
					ImGui::SetItemDefaultFocus(); // 聚焦
				}
			}
			ImGui::EndListBox();
		}

		ImGui::ColorEdit4("MyColorPanel_1", color, ImGuiColorEditFlags_::ImGuiColorEditFlags_AlphaBar);
		ImGui::End();

		ImGui::Render(); // 结束当前帧, 生成绘制数据(顶点, 索引缓冲等), 在所有UI 控件定义完成后调用
		
		// 将ImGui的绘制数据提交给OpenGL后端进行实际渲染
		//  GetDrawData(), 获取当前帧的绘制数据
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(pWindow); // 交换前后缓冲区
		glfwPollEvents(); // 轮询 - glfw 与 窗口通信
	}

	// 清理资源
	ImGui_ImplOpenGL3_Shutdown(); // 关闭OpenGL后端, 释放资源
	ImGui_ImplGlfw_Shutdown();// 关闭 GLFW 后端, 移除回调函数
	ImGui::DestroyContext();// 销毁 ImGui 上下文
	glfwDestroyWindow(pWindow); // 销毁窗口
	glfwTerminate();			// 终止GLFW
}