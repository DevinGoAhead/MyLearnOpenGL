#ifndef __GLOBAL
#define __GLOBAL

#include "glad/glad.h"
#include "glfw/glfw3.h"

#include <iostream>
#include <string>
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <vector>

using uint = unsigned int;

// window
int wndWidth = 0, wndHeight = 0;

// 错误处理回调函数
void error_callback(int error_code, const char* description){
	std::cerr << "GLFW Error: " << description << std::endl;
	exit(1);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
	switch (key) {
		case GLFW_KEY_ESCAPE: {
			if(action == GLFW_PRESS)
				glfwSetWindowShouldClose(window, GL_TRUE);
			break;
		}
		default:{
		}
	}
}

void viewport_size_callback(GLFWwindow* window, int width, int height) {
	//glfwGetFramebufferSize(window, &wndWidth, &wndHeight);
	wndWidth = width, wndHeight = height;
	glViewport(0, 0, wndWidth, height);
}

#endif