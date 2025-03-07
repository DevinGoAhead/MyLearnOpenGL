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

std::vector<float> planeVertices = {
	// positions            // normals         // texcoords
	 10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,  10.0f,  0.0f,
	-10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
	-10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,   0.0f, 10.0f,

	 10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,  10.0f,  0.0f,
	-10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,   0.0f, 10.0f,
	10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,  10.0f, 10.0f
};

using uint = unsigned int;

// window
int wndWidth = 0, wndHeight = 0;

wxy::Camera camera({0.f, 0.f, 3.f});//camera, pos

// time
float curTime = 0.f, lastTime = 0.f, perFrameTime = 0.f;

//cursor
float xLast = wndWidth / 2.f, yLast = wndHeight / 2.f; // 初始值为视口中心

// 错误处理回调函数
void error_callback(int error_code, const char* description){
	std::cerr << "GLFW Error: " << description << std::endl;
	exit(1);
}

bool isBlinn = true, blinnKeyPressed = false;
bool isGamma = true, gammaKeyPressed = false;
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
	switch (key) {
		case GLFW_KEY_ESCAPE: {
			if(action == GLFW_PRESS)
				glfwSetWindowShouldClose(window, GL_TRUE);
			break;
		}
		case GLFW_KEY_W : {// 前
			camera.ProcessKeyBoard(wxy::Front, perFrameTime);
			break;
		}
		case GLFW_KEY_S : {// 后
			camera.ProcessKeyBoard(wxy::Back, perFrameTime);
			break;

		}
		case GLFW_KEY_A : {// 左
			camera.ProcessKeyBoard(wxy::Left, perFrameTime);
			break;
		}
		case GLFW_KEY_D : {// 右
			camera.ProcessKeyBoard(wxy::Right, perFrameTime);
			break;
		}
		case GLFW_KEY_B : { // 切换 Blinn-Phong 与 Phong
			// 确保按下 B 键 只会生效一次, 即使你按了1秒甚至更长
			// 否则当你按下 B后, 每一帧都会切换
			if(action == GLFW_PRESS && !blinnKeyPressed) {
				isBlinn = !isBlinn;
				blinnKeyPressed = true;
			}
			if(action == GLFW_RELEASE) {blinnKeyPressed = false;}
		}
		case GLFW_KEY_G : { // 是否 gamma 矫正
			if(action == GLFW_PRESS && !gammaKeyPressed) {
				isGamma = !isGamma;
				gammaKeyPressed = true;
			}
			if(action == GLFW_RELEASE) {gammaKeyPressed = false;}
		}

		default:{
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

void viewport_size_callback(GLFWwindow* window, int width, int height) {
	//glfwGetFramebufferSize(window, &wndWidth, &wndHeight);
	wndWidth = width, wndHeight = height;
	glViewport(0, 0, wndWidth, height);
}

void GetError() {
	GLenum err = glGetError();
	if (err != GL_NO_ERROR) {
		std::cerr << "OpenGL Error: " << err << std::endl;
	}
}

#endif