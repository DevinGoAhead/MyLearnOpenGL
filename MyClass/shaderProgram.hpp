
#ifndef MY_SHADER
#define MY_SHADER

#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <type_traits>
#include <ctime>

#ifdef DEBUG
#include <filesystem>
namespace fs = std::filesystem;
#endif
namespace wxy{
	class ShaderProgram
	{
	public:
		ShaderProgram(const std::string& vertexShaderFilePath, const std::string& fragShaderFilePath);
		void Use();//激活 shader program
		
		// 设定 uniform 变量的值(可变参数模板)
		// 后面发现很难穷举针对所有类型的函数, 舍弃了该方法, date: 02.21, 好像还能补救一下
		template<typename T, typename... Args>
		void SetUniform(const std::string& uniformName, T value, Args... args);
		template<typename T>
		void SetUniformv(const std::string& uniformName, GLsizei count, GLboolean transpose, T& value);
		GLuint ID() {return _shaderProgram;}
	private:
		void FileToString(const std::string& FilePath, std::string& strDestination);// 将文件内容 "拷贝" 至字符串
		void isCompileSuccess(GLuint shader);//检测着色器编译是否成功, 并获取链接日志
		void isLinkSuccess(GLuint shaderProgram);//检测着色程序链接是否成功, 并获取链接日志
	private:
		GLuint _shaderProgram;
	};

	ShaderProgram::ShaderProgram(const std::string& vertexShaderFilePath, const std::string& fragmentShaderFilePath)
	{
		std::string vertexShaderCode, fragmentShaderCode;//字符串用于存储着色器源码
		FileToString(vertexShaderFilePath,  vertexShaderCode);
		FileToString(fragmentShaderFilePath, fragmentShaderCode);
		
		// #ifdef DEBUG
		// std::cout << "vertexShaderCode: " << vertexShaderCode<<std::endl;
		// std::cout << "fragmentShaderCode: " <<fragmentShaderCode<<std::endl;
		// #endif

		/*创建并编译着色器*/
		GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);		// 创建顶点着色器
		GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER); // 创建片段着色器

		// 使用目标着色器源码覆盖着色器中旧的源码(因为现在着色器是新建的,所以这里也可以说是填充源码)
		const char* c_vertexShader = vertexShaderCode.c_str();
		const char* c_fragmentShaderCode = fragmentShaderCode.c_str();
		glShaderSource(vertexShader, 1, &c_vertexShader, NULL);
		glShaderSource(fragmentShader, 1, &c_fragmentShaderCode, NULL);

		// 编译着色器并检测是否编译成功
		glCompileShader(vertexShader);
		isCompileSuccess(vertexShader);
		
		glCompileShader(fragmentShader);
		isCompileSuccess(fragmentShader);

		/*创建程序对象*/
		_shaderProgram = glCreateProgram();
		glAttachShader(_shaderProgram, vertexShader);
		glAttachShader(_shaderProgram, fragmentShader);
		glLinkProgram(_shaderProgram);			   // 链接
		isLinkSuccess(_shaderProgram);
	}

	void ShaderProgram::Use()
	{
		glUseProgram(_shaderProgram);
	}

	template<typename T, typename... Args>
	void ShaderProgram::SetUniform(const std::string& uniformName, T value, Args... args)
	{
		GLuint uniformLocation = glGetUniformLocation(_shaderProgram, uniformName.c_str());
		// 这里至少要求有一个uniform分量不在参数包中,而去对应 T, 用于确定 uniform 的值得类型
		if constexpr (std::is_same_v<T, GLuint>)
		{
			if constexpr (sizeof...(args) == 0){glUniform1ui(uniformLocation, value, args...);}
			else if  constexpr (sizeof...(args) == 1){glUniform2ui(uniformLocation, value, args...);}
			else if constexpr (sizeof...(args) == 2){glUniform3ui(uniformLocation, value, args...);}
			else if constexpr (sizeof...(args) == 3){glUniform4ui(uniformLocation, value, args...);}
		}
		else if constexpr (std::is_same_v<T, GLint>)
		{
			if constexpr (sizeof...(args) == 0){glUniform1i(uniformLocation, value, args...);}
			else if constexpr (sizeof...(args) == 1){glUniform2i(uniformLocation, value, args...);}
			else if constexpr (sizeof...(args) == 2){glUniform3i(uniformLocation, value, args...);}
			else if constexpr (sizeof...(args) == 3){glUniform4i(uniformLocation, value, args...);}
		}
		else if constexpr (std::is_same_v<T, GLfloat>)
		{
			if constexpr (sizeof...(args) == 0){glUniform1f(uniformLocation, value, args...);}
			else if constexpr (sizeof...(args) == 1){glUniform2f(uniformLocation, value, args...);}
			else if constexpr (sizeof...(args) == 2){glUniform3f(uniformLocation, value, args...);}
			else if constexpr (sizeof...(args) == 3){glUniform4f(uniformLocation, value, args...);}
		}
	}

	template<typename T>
	void ShaderProgram::SetUniformv(const std::string& uniformName, GLsizei count, GLboolean transpose, T& value)
	{
		GLuint uniformLocation = glGetUniformLocation(_shaderProgram, uniformName.c_str());
		if constexpr (std::is_same_v<T, glm::vec3>){glUniform3fv(uniformLocation, count, transpose, glm::value_ptr(value));}
		if constexpr (std::is_same_v<T, glm::mat4>){glUniformMatrix4fv(uniformLocation, count, transpose, glm::value_ptr(value));}
	}
	void ShaderProgram::FileToString(const std::string& FilePath, std::string& strDestination)
	{
		// #ifdef DEBUG
		// std::cout << "Current path: " << fs::current_path() << std::endl;
		// std::cout << "FilePath: " << FilePath << std::endl;
		// #endif
		std::ifstream iFileStrm;
		iFileStrm.exceptions(std::ios_base::failbit | std::ios_base::badbit);//设置抛出异常的类型
		try
		{
			iFileStrm.open(FilePath);
			std::ostringstream oStrStrm;
			oStrStrm << iFileStrm.rdbuf();
			strDestination = oStrStrm.str();
			iFileStrm.close();
		}
		catch(const std::ifstream::failure& ex)
		{
			std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << ex.what() << std::endl;
		}
	}

	void ShaderProgram::isCompileSuccess(GLuint shader)
	{
		GLint compileStatus = 0;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus); // 获取着色器编译状态
		// 如果编译失败,报错退出
		if (!compileStatus)
		{
			std::cout << "ERROR::SHADER::COMPILE_FAILED\n"
					  << std::endl;
			//日志
			GLchar log[1024];
			glGetShaderInfoLog(shader, 1023, NULL, log); // 字符串存储编译日志

			// 将日志存储到本地
			time_t now;
			time(&now);
			struct tm *local = localtime(&now);
			char fileName[64];
			// sprintf(fileName, "compile_log_%s",ctime(&now));//文件格式为compile_log_ + 时间
			strftime(fileName, sizeof(fileName), "compile_log_%Y-%m-%d.txt", local);

			FILE *logFile = fopen(fileName, "a");
			fprintf(logFile, "%s\n", log);
			fclose(logFile);
			exit(EXIT_FAILURE);
		}
	}

	void ShaderProgram::isLinkSuccess(GLuint shaderProgram)
	{
		// 如果编译失败,报错,打印日志, 退出
		GLint linkStatus = 0;
		glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linkStatus); // 获取程序链接状态
		if (!linkStatus)
		{
	  		std::cout << "ERROR::SHADER::LINK_FAILED\n"
					  << std::endl;
			//日志
			GLchar log[1024];
			glGetProgramInfoLog(shaderProgram, 1023, NULL, log); // 字符串存储链接日志

			// 将日志存储到本地
			time_t now;
			time(&now);
			struct tm *local = localtime(&now);
			char fileName[64];
			// sprintf(fileName, "link_log_%s",ctime(&now));//文件格式为compile_log_ + 时间
			strftime(fileName, sizeof(fileName), "link_log_%Y-%m-%d.txt", local);

			FILE *logFile = fopen(fileName, "a");
			fprintf(logFile, "%s\n", log);

			fclose(logFile);
			exit(EXIT_FAILURE);
		}
	}
}
#endif
