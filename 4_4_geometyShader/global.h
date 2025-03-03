#ifndef __GLOBAL
#define __GLOBAL

// #define STB_IMAGE_IMPLEMENTATION
// #include "stb_image.h" // 加载纹理图像
#include "glad/glad.h"
#include "glfw/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <vector>
#include <map>
#include <functional>
#include <filesystem>
#include "../MyClass/shaderProgram.hpp"
#include "../MyClass/camera.hpp"
#include "../MyClass/model.hpp"

using uint = unsigned int;

// window
int wndWidth, wndHeight;

wxy::Camera camera({0.f, 2.f, 10.f});//camera, pos

// time
float curTime = 0.f, lastTime = 0.f, perFrameTime = 0.f;

//cursor
float xLast = wndWidth / 2.f, yLast = wndHeight / 2.f; // 初始值为视口中心

// 错误处理回调函数
void error_callback(int error_code, const char* description){
	std::cerr << "GLFW Error: " << description << std::endl;
	exit(1);
}

void viewport_size_callback(GLFWwindow* window, int width, int height) { glViewport(0, 0, width, height); }

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
	switch (key)
	{
		case GLFW_KEY_ESCAPE:
		{
			if(action == GLFW_PRESS)
				glfwSetWindowShouldClose(window, GL_TRUE);
			break;
		}
		case GLFW_KEY_W : // 前
		{
			camera.ProcessKeyBoard(wxy::Front, perFrameTime);
			break;
		}
		case GLFW_KEY_S : // 后
		{
			camera.ProcessKeyBoard(wxy::Back, perFrameTime);
			break;

		}
		case GLFW_KEY_A : // 左
		{
			camera.ProcessKeyBoard(wxy::Left, perFrameTime);
			break;
		}
		case GLFW_KEY_D : // 右
		{
			camera.ProcessKeyBoard(wxy::Right, perFrameTime);
			break;
		}
		default:
		{

		}
	}
}

bool first = true;
void cursor_callback(GLFWwindow* window, double xPos, double yPos) {
	// 计算偏移值
	// 当光标向上移动时会得到一个 < 0 的 dy, 如果 _pitch += dy 会导致 _pitch 减小, 这正好与我们的目标相反, 故 dy 取反
	float dx = xPos - xLast, dy = yLast - yPos;
	if(first) {dx = 0, dy =0, first = false;}
	camera.ProcessMouseMove(dx, dy);
	xLast = xPos, yLast = yPos;
}

// 多数鼠标无法在 x 方向滚动, 故暂忽略 xoffset
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	// 通常滚轮向下是一个放大的操作, 而滚轮向下会导致一个 > 0 的 yoffset, 这与我们预期一致
	camera.ProcessMouseScroll(xoffset,  yoffset);
}
#endif