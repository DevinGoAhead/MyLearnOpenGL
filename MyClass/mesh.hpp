// c++ 的结构体的内存分布是连续的

#ifndef __MESH
#define __MESH

#include "../MyClass/shaderProgram.hpp"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "glad/glad.h"
#include "glfw/glfw3.h"

#include <string>
#include <vector>

namespace wxy{
	using uint = unsigned int;
	// 顶点
	struct Vertex{
		glm::vec3 _position;
		glm::vec3 _normal;
		glm::vec2 _texCoord;
	};

	// 纹理
	struct Texture{
		uint _id; // 纹理ID
		std::string _typeName; // 纹理类型
		std::string _fileName; // 文件名
	};

	// Mesh
	class Mesh{
	public:
		Mesh(const std::vector<Vertex>& vertices, const std::vector<uint>& indices, const std::vector<Texture>& textures);
		~Mesh();
		void Draw(ShaderProgram shaderProgram);
	private:
		void SetupMesh();
	private:
		std::vector<Vertex> _vertices; // 实际的顶点数据
		std::vector<uint> _indices; // 顶点索引
		std::vector<Texture> _textures; // 纹理

		uint _VBO, _EBO, _VAO;
	};

	//public
	Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<uint>& indices, const std::vector<Texture>& textures) 
		: _vertices(vertices), _indices(indices), _textures(textures) {
		SetupMesh();
	}
	Mesh::~Mesh() {
		// glDeleteBuffers(1,&_VBO);
		// glDeleteBuffers(1,&_EBO);
		// glDeleteVertexArrays(1, &_VAO);
	}
	void Mesh::Draw(ShaderProgram shaderProgram) {
		uint size = _textures.size();

		uint textureDiffuseNr = 0, textureSpecularNr = 0, textureReflectionNr = 0;
		std::string number;
		//GLenum target;
		for(uint i = 0; i < size; ++i){
			// _typeName: textureDiffuse textureSpecular textureReflection
			std::string texTypeName = _textures[i]._typeName;
			glActiveTexture(GL_TEXTURE0 + i);
			//target = (texTypeName == "textureReflection" ? GL_TEXTURE_2D : GL_TEXTURE_2D);
			//glBindTexture(target, _textures[i]._id); // 将纹理绑定到目标类型
			glBindTexture(GL_TEXTURE_2D, _textures[i]._id); // 将纹理绑定到目标类型
			if(texTypeName == "textureDiffuse") {number = std::to_string(textureDiffuseNr++);}
			else if(texTypeName == "textureSpecular") {number = std::to_string(textureSpecularNr++);}
			else if(texTypeName == "textureReflection") {number = std::to_string(textureReflectionNr++);}
			shaderProgram.SetUniform(("material_." + texTypeName + number).c_str(), i); // 将纹理单元的索引传递给 GPU 中对应的 Sampler 对象
		}
		shaderProgram.SetUniform("material_.shininess",64); // 这里暂先硬编码, 有需要再优化
		glActiveTexture(GL_TEXTURE0); // 激活状态恢复到纹理单元 0

		// draw
		glBindVertexArray(_VAO);
		glDrawElements(GL_TRIANGLES, static_cast<uint>(_indices.size()), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0); // 解绑
	}

	//private
	void Mesh::SetupMesh() {
		glGenBuffers(1, &_VBO); // 创建一个 buffer, 绑定 _VBO, 将来将作为 vertex buffer
		glGenBuffers(1, &_EBO); // 创建一个 buffer, 绑定 _EBO, 将来将作为 element(index) buffer
		glGenVertexArrays(1, &_VAO); // 创建一个 vertex array, 绑定 _VAO

		glBindVertexArray(_VAO); // 开始记录
		glBindBuffer(GL_ARRAY_BUFFER, _VBO); // 告知 GPU, _VBO 绑定的是 vertex buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _EBO); // 告知 GPU, _EBO 绑定的是 element(index) buffer
		
		// _vertices.size() * sizeof(Vertex) instead of sizeof(_vertices)
		// sizeof(_vertices), including some other data, member value, such as start pointer, finish pointer and end pointer
		glBufferData(GL_ARRAY_BUFFER, _vertices.size() * sizeof(Vertex), &_vertices[0], GL_STATIC_DRAW); // GPU 中开辟内存存储 _vertices
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, _indices.size() * sizeof(uint), &_indices[0], GL_STATIC_DRAW); // GPU 中开辟内存存储 _indices

		// 告知 GPU 如何解析顶点数据
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, _position)); // 位置坐标
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, _normal)); // 法线坐标
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, _texCoord)); // 纹理坐标
		glEnableVertexAttribArray(0), glEnableVertexAttribArray(1), glEnableVertexAttribArray(2);

		glBindVertexArray(0);// 解绑 _VAO, 停止记录
	}
}
#endif