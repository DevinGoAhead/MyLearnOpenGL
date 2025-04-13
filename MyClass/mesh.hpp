// c++ 的结构体的内存分布是连续的

#ifndef _MESH
#define _MESH

#include "../MyClass/shaderProgram.hpp"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <glm/gtx/string_cast.hpp>

#include "glad/glad.h"
#include "glfw/glfw3.h"

#include <string>
#include <vector>
#include <array>
#include <utility>
#include <algorithm>

const uint MaxInfluenceBone = 4;

// 临时存储与顶点关联的骨骼信息, 经过处理后拷贝到 Vertex 中
class BoneInfluenceSet {
public:
	void SortByWeight();
	void NormalizeWeights();
	uint Size();
	void Resize(uint newSize);
	void PushBack(uint boneID, float weight);
	void CopyToArray(std::array<int, MaxInfluenceBone> &boneIDs, std::array<float, MaxInfluenceBone> &weights);
private:
	std::vector<std::pair<uint, float>> _weightsPerBone; // ID, weight
};

//BoneWeight(uint vertexID, uint boneID, float weight) : _vertexID(vertexID),  {}
// 按权重由大到小排序
void BoneInfluenceSet::SortByWeight() {
	std::sort(_weightsPerBone.begin(), _weightsPerBone.end(),
			[](std::pair<uint, float> _boneIDweightLt, std::pair<uint, float> _boneIDweightRt)->bool {
				return _boneIDweightLt.second > _boneIDweightRt.second;});
}

// 本例假设每个顶点最多受到 MaxInfluenceBone 个骨骼的影响
// 如果超过 MaxInfluenceBone 个, 则截断, 这种情况这导致 _weights 之和 < 1, 针对这种情况做归一化
// 在外部判断 _weightsPerBone.size > MaxInfluenceBone? 同时resize(MaxInfluenceBone), 再调用  NormalizeWeights()
void BoneInfluenceSet::NormalizeWeights() {
	float totalWeight = 0.f;
	for(const auto& _boneIDweight : _weightsPerBone) {totalWeight += _boneIDweight.second;}
	if(totalWeight <= 0.f ) {
		if(_weightsPerBone.empty()) {std::cout << "No bones influence current vertex !" << std::endl;}
		else {std::cout << "Bones' weights are wrong !" << std::endl;}
	}
	else {
		for(auto& _boneIDweight : _weightsPerBone) {
			_boneIDweight.second /=  totalWeight;
		}
	}
}

uint BoneInfluenceSet::Size() {return _weightsPerBone.size();}
void BoneInfluenceSet:: Resize(uint newSize) {_weightsPerBone.resize(newSize);}
void BoneInfluenceSet::PushBack(uint boneID, float weight) {_weightsPerBone.emplace_back(boneID, weight);}
void BoneInfluenceSet::CopyToArray(std::array<int, MaxInfluenceBone> &boneIDs, std::array<float, MaxInfluenceBone> &weights) {
	uint size = _weightsPerBone.size(); // 一定 < MaxInfluenceBone
	for(uint i = 0; i < MaxInfluenceBone; ++i){
		if(i < size) {
			boneIDs[i] = _weightsPerBone[i].first;
			weights[i] = _weightsPerBone[i].second;
		}
		else {
			boneIDs[i] = -1;
			weights[i] = 0.f;
		}
	}
}

// 顶点
struct Vertex{
	glm::vec3 _position;
	glm::vec3 _normal;
	glm::vec2 _texCoord;
	glm::vec3 _tangent;
	std::array<int, MaxInfluenceBone> _boneIDs;
	std::array<float, MaxInfluenceBone> _weights;

	//LearnOpenGL 的逻辑
	// void SetBoneIDs_Weights(int boneID, float weight){
	// 	for(uint i = 0; i < MaxInfluenceBone; ++i) {
	// 		if(_boneIDs[i] != -1) {
	// 			_boneIDs[i] = boneID;
	// 			_weights[i] = weight;
	// 		}
	// 	}
	// }
};

// 纹理
struct Texture{
	uint _id; // 纹理ID
	std::string _typeName; // 纹理类型
	std::string _fileName; // 文件名
};

namespace wxy{
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

		uint textureDiffuseNr = 0, textureSpecularNr = 0, textureReflectionNr = 0, textureNormalNr = 0;
		std::string number;
		//GLenum target;
		for(uint i = 0; i < size; ++i){
			// _typeName: textureDiffuse textureSpecular textureReflection
			std::string texTypeName = _textures[i]._typeName;
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, _textures[i]._id); // 将纹理绑定到目标类型
			if(texTypeName == "textureDiffuse") {number = std::to_string(textureDiffuseNr++);}
			else if(texTypeName == "textureSpecular") {number = std::to_string(textureSpecularNr++);}
			else if(texTypeName == "textureReflection") {number = std::to_string(textureReflectionNr++);}
			else if(texTypeName == "textureNormal") {number = std::to_string(textureNormalNr++);}
			shaderProgram.SetUniform(("uMaterial." + texTypeName + number).c_str(), i); // 将纹理单元的索引传递给 GPU 中对应的 Sampler 对象
		}
		//shaderProgram.SetUniform("material_.shininess",64); // 这里暂先硬编码, 有需要再优化
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
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, _texCoord)); // 材质纹理
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, _tangent)); // 法线纹理
		
		glVertexAttribIPointer(4, MaxInfluenceBone, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, _boneIDs)); // 影响骨骼的 ID 数组, 分量 4
		glVertexAttribPointer(5, MaxInfluenceBone, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, _weights)); // 影响骨骼的 weight 数组, 分量 4
		
		glEnableVertexAttribArray(0); glEnableVertexAttribArray(1); glEnableVertexAttribArray(2); glEnableVertexAttribArray(3);
		glEnableVertexAttribArray(4); glEnableVertexAttribArray(5);

		glBindVertexArray(0);// 解绑 _VAO, 停止记录
	}
}
#endif