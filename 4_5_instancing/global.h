#ifndef __GLOBAL
#define __GLOBAL

#include "glad/glad.h"
#include "glfw/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <vector>
#include <random>
#include "../MyClass/shaderProgram.hpp"
#include "../MyClass/camera.hpp"
#include "../MyClass/model.hpp"

#define GLM_ENABLE_EXPERIMENTAL // 必须定义在包含GLM头文件之前
#include <glm/gtx/string_cast.hpp> // 包含字符串转换功能

std::vector<float> quadVertices = {
	// 位置 // 颜色
	-0.05f, 0.05f, 1.0f, 0.0f, 0.0f,
	0.05f, -0.05f, 0.0f, 1.0f, 0.0f,
	-0.05f, -0.05f, 0.0f, 0.0f, 1.0f,
	-0.05f, 0.05f, 1.0f, 0.0f, 0.0f,
	0.05f, -0.05f, 0.0f, 1.0f, 0.0f, 
	0.05f, 0.05f, 0.0f, 1.0f, 1.0f 
	}; 

using uint = unsigned int;

// window
int wndWidth = 0, wndHeight = 0;

wxy::Camera camera({-2.f, 5.f, 20.f});//camera, pos

// time
float curTime = 0.f, lastTime = 0.f, perFrameTime = 0.f;

//cursor
float xLast = wndWidth / 2.f, yLast = wndHeight / 2.f; // 初始值为视口中心

// 错误处理回调函数
void error_callback(int error_code, const char* description){
	std::cerr << "GLFW Error: " << description << std::endl;
	exit(1);
}

void viewport_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height); 
	wndWidth = width, wndHeight = height;
}

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

// 生成一个随机数
float Random(float start, float end) {
	std::random_device rd; // 随机数生成器, 用于生成一个随机数种子
	std::mt19937 gen(rd()); // 这是实际生成随机数的引擎
	std::uniform_real_distribution<float> distriute(start, end);
	return distriute(gen);
}
void GetError() {
	GLenum err = glGetError();
	if (err != GL_NO_ERROR) {
		std::cerr << "OpenGL Error: " << err << std::endl;
	}
}

#endif