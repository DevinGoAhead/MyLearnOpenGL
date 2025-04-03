#ifndef __GLOBAL
#define __GLOBAL

#include "glad/glad.h"
#include "glfw/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h" // 需包含 stb_image_write.h
#include <vector>
#include <random>
#include <functional>
#include <cmath>
#include "../MyClass/shaderProgram.hpp"
#include "../MyClass/camera.hpp"
#include "../MyClass/model.hpp"
#include "../MyClass/sphere.hpp"

using uint = unsigned int;
std::vector<float> cubeVertices{
		// back face
		-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
		 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
		 1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
		 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
		-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
		-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
		// front face
		-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
		 1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
		 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
		 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
		-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
		-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
		// left face
		-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
		-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
		-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
		-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
		-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
		-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
		// right face
		 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
		 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
		 1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
		 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
		 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
		 1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
		// bottom face
		-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
		 1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
		 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
		 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
		-1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
		-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
		// top face
		-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
		 1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
		 1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
		 1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
		-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
		-1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
};

std::vector<float> quadVertices = {
	// positions        // texture Coords
	-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
	-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
	 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
	 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
};
// window
int wndWidth = 0, wndHeight = 0;

wxy::Camera camera({-4.f, 4.f, 10.f});//camera, pos

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
float exposure = 1.f; // 纹理偏移比例系数
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
			break;
		}
		case GLFW_KEY_G : { // 是否 gamma 矫正
			if(action == GLFW_PRESS && !gammaKeyPressed) {
				isGamma = !isGamma;
				gammaKeyPressed = true;
			}
			if(action == GLFW_RELEASE) {gammaKeyPressed = false;}
			break;
		}
		case GLFW_KEY_K : { //增加 exposure 
			exposure += 0.01;
			break;
		}
		case GLFW_KEY_J : {//降低 exposure 
			exposure -= 0.01;
			if(exposure < 0.00001) exposure = 0.f;
			break;
		}
		default:{
		}
	}
}

bool first = true;
void cursor_callback(GLFWwindow* window, double xPos, double yPos) {
	if (ImGui::GetIO().WantCaptureMouse) {
        return; // ImGui正在操作，阻断相机响应
    }
	// 计算偏移值
	// 当光标向上移动时会得到一个 < 0 的 dy, 如果 _pitch += dy 会导致 _pitch 减小, 这正好与我们的目标相反, 故 dy 取反
	float dx = xPos - xLast, dy = yLast - yPos;
	if(first) {dx = 0, dy =0, first = false;}
	camera.ProcessMouseMove(dx, dy);
	xLast = xPos, yLast = yPos;
}

// 多数鼠标无法在 x 方向滚动, 故暂忽略 xoffset
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	if (ImGui::GetIO().WantCaptureMouse) {
        return; // ImGui正在操作，阻断相机响应
    }
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

// 读取 Cube Map 纹理并保存为 PNG
void SaveTexToPNG(GLuint textureID, int mipLevel, std::string path) {
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    // 获取纹理大小
    int width, height;
    glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X, mipLevel, GL_TEXTURE_WIDTH, &width);
    glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X, mipLevel, GL_TEXTURE_HEIGHT, &height);

    if (width == 0 || height == 0) {
        std::cerr << "size of  texture error!" << std::endl;
        return;
    }

    std::vector<unsigned char> data(width * height * 3); // 3 通道 (RGB)

    for (int i = 0; i < 6; ++i) {
        glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, mipLevel, GL_RGB, GL_UNSIGNED_BYTE, data.data());

        // 使用 stb_image_write 保存为 PNG
        if (!stbi_write_png((path + std::to_string(mipLevel) + "_" + "face_" + std::to_string(i) + ".png").c_str(), 
		width, height, 3, data.data(), width * 3)) {
            std::cerr << "Error::stbi_write_png!" << std::endl;
        }
    }

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0); // 解绑纹理
}

// 读取 TEXTURE_2D 纹理并保存为 PNG
// 
void SaveTexToPNG(GLuint textureID, std::string path) {
    glBindTexture(GL_TEXTURE_2D, textureID);

    int width, height;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);

    // 使用浮点数组读取数据
    std::vector<float> data(width * height * 2); // 2通道
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RG, GL_FLOAT, data.data());

    // 转换为字节数据（0-255）
    std::vector<unsigned char> byteData(width * height * 2);
    for (int i = 0; i < width * height * 2; ++i) {
        // 假设浮点值范围在[0,1]，映射到[0,255]
        byteData[i] = static_cast<unsigned char>(data[i] * 255.0f);
    }

	std::vector<unsigned char> rgbData(width * height * 3);
	for (int i = 0; i < width * height; ++i) {
	    rgbData[i * 3 + 0] = byteData[i * 2 + 0]; // R
	    rgbData[i * 3 + 1] = byteData[i * 2 + 1]; // G
	    rgbData[i * 3 + 2] = 0;                   // B 通道填充 0
	}
    // 保存为2通道PNG
    if (!stbi_write_png((path + "BRDF_1.png").c_str(), 
        width, height, 3, rgbData.data(), width * 3)) {
        std::cerr << "Error::stbi_write_png!" << std::endl;
    }

    glBindTexture(GL_TEXTURE_2D, 0);
}
#endif