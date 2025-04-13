#ifndef _MODEL
#define _MODEL

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h" // 加载纹理图像
#endif

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "mesh.hpp"
#include "assimpGLMHelpers.hpp"
#include <iostream>
#include <map>
#include <cstring>
#include <type_traits>

struct BoneInfo {
	BoneInfo(int id = -1, glm::mat4 offset = glm::mat4(1.f))
	: _id(id), _offset(offset) {}

	int _id; // bone 的下标(索引)
	glm::mat4 _offset; // 从骨骼空间到模型空间
};

namespace wxy {
	class Model {
	public:
		Model(const std::string& pathName);
		void Draw(wxy::ShaderProgram shaderProgram);
		std::map<std::string, BoneInfo>& GetBoneInfoMap();
		uint& GetBoneCount();
		//~Model();
	private:
		void LoadModel(const std::string& pathName); // 加载模型, pathName 模型文件的路径
		void ProcessNode(aiNode* assimpNode, const aiScene* assimpScene); // 遍历所有节点 及其 子节点
		Mesh ProcessMesh(aiMesh* assimpMesh, const aiScene* assimpScene);
		std::vector<Texture> loadMaterialTextures(aiMaterial* assimpMat, aiTextureType assimpTexType, const std::string& texTypeName);
		//void LoadMeshBones(std::vector<Vertex>& vertices, aiMesh* assimpMesh, const aiScene* assimpScene);
		void LoadMeshBones(std::vector<BoneInfluenceSet>& boneIDweights, aiMesh* assimpMesh, const aiScene* assimpScene);
		uint TextureFromFile(const std::string& assimpTexFileName/*, const std::string& texTypeName*/);
	private:
		std::vector<Mesh> _meshes; // 存放加载的所有mesh
		std::map<std::string, Texture> _texturesLoaded; // 已经加载过的纹理, string 是纹理的名称
		std::map<std::string, BoneInfo> _boneInfoMap; // 已经加载过的骨骼, string 是骨骼的名称
		uint _boneCount; //所有 mesh, 累计加载的骨骼的数量
		std::string _directory; // 不含文件名的模型文件的路径
	};

	//public
	Model::Model(const std::string& pathName) : _boneCount(0) {
		LoadModel(pathName);
	}

	void Model::Draw(ShaderProgram shaderProgram) {
		uint size = _meshes.size();
		//std::cout << size << std::endl;
		for(uint i = 0; i <  size; ++i) {
			_meshes[i].Draw(shaderProgram);
		}
	}

	std::map<std::string, BoneInfo>& Model::GetBoneInfoMap() {return _boneInfoMap;}
	uint& Model::GetBoneCount() {return _boneCount;}

	//private
	void Model::LoadModel(const std::string& absolutePathName) {
		Assimp::Importer import;
		// 将非三角形处理为三角形 | 反转纹理图像的 y 坐标 | 强制统一为 CCW 顺序 | 自动计算 BT
		const aiScene *assimpScene = import.ReadFile(absolutePathName, 
				aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_FlipWindingOrder | aiProcess_CalcTangentSpace); 
		// 只要有一个为假即判断为失败
		if(!assimpScene || (assimpScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !assimpScene->mRootNode)) {
			std::cerr << "ASSIMP::ERROR::" << import.GetErrorString() << std::endl;
		}
		
		_directory = absolutePathName.substr(0, absolutePathName.find_last_of('/')); // 使用 [0, 最后一个/) 之间的内容创建一个子字符串, 其实就是不含文件名的路径
		ProcessNode(assimpScene->mRootNode, assimpScene);
	}

	void Model::ProcessNode(aiNode* assimpNode, const aiScene* assimpScene)
	{
		// 对于每一个节点, 都遍历它所有的 mesh (的索引)
		uint numMeshes = assimpNode->mNumMeshes;
		for(uint i = 0; i < numMeshes; ++i) {
			aiMesh* assimpMesh = assimpScene->mMeshes[assimpNode->mMeshes[i]];
			_meshes.emplace_back(ProcessMesh(assimpMesh, assimpScene));
		}

		// 遍历所有子节点
		for(uint i = 0; i < assimpNode->mNumChildren; ++i) {ProcessNode(assimpNode->mChildren[i], assimpScene);}
	}

	Mesh Model::ProcessMesh(aiMesh* assimpMesh, const aiScene* assimpScene) {
		std::vector<Vertex> vertices;
		std::vector<uint> indices;
		std::vector<Texture> textures;
		// 每个 mesh 会有多个顶点
		uint numVertices = assimpMesh->mNumVertices;
		for(uint i = 0; i < numVertices; ++i) {
			glm::vec3 vec3Temp;
			Vertex vertex;
			vec3Temp.x = assimpMesh->mVertices[i].x;
			vec3Temp.y = assimpMesh->mVertices[i].y;
			vec3Temp.z = assimpMesh->mVertices[i].z;
			vertex._position = vec3Temp;

			vec3Temp.x = assimpMesh->mNormals[i].x;
			vec3Temp.y = assimpMesh->mNormals[i].y;
			vec3Temp.z = assimpMesh->mNormals[i].z;
			vertex._normal = vec3Temp;

			glm::vec2 vec2Temp;
			// Assimp 最多允许一个顶点上有 8 组纹理坐标, 但本例现在仅使用第一组
			if(assimpMesh->mTextureCoords[0]) { // 数组不为空
				vec2Temp.x = assimpMesh->mTextureCoords[0][i].x;
				vec2Temp.y = assimpMesh->mTextureCoords[0][i].y;
				vertex._texCoord = vec2Temp;
			}
			else {vertex._texCoord = {0.f, 0.f};}
			// T 的 下标直接与顶点对应
			// 考虑现在模型大多是动态,因此在导入前就完成正交化必要性不大, 渲染时还是得重新计算
			vec3Temp.x = assimpMesh->mTangents[i].x;
			vec3Temp.y = assimpMesh->mTangents[i].y;
			vec3Temp.z = assimpMesh->mTangents[i].z;
			vertex._tangent = vec3Temp;
			
			// boneIDs 和 boneWeights 的初始化放在了 class BoneInfluenceSet 中

			vertices.push_back(vertex);
		}

		// 所有顶点初始化完成, 顶点ID 已确定

		std::vector<BoneInfluenceSet> BoneInfluenceSet; // 临时存储骨骼id 和 weight, 与 vertices 一一对应
		uint sizeVertices = vertices.size();
		BoneInfluenceSet.resize(sizeVertices);
		
		LoadMeshBones(BoneInfluenceSet, assimpMesh, assimpScene);
		for(uint i = 0; i < sizeVertices; ++i) {
			auto& boneInfluence = BoneInfluenceSet[i];
			if(boneInfluence.Size() > MaxInfluenceBone) {
				boneInfluence.SortByWeight();
				boneInfluence.Resize(MaxInfluenceBone);
				boneInfluence.NormalizeWeights();
			}
			boneInfluence.CopyToArray(vertices[i]._boneIDs, vertices[i]._weights);
		}

		// 每个 mesh 会有多个索引, 其实就是顶点索引, 指定了由哪几个点组成当前face
		uint numFaces = assimpMesh->mNumFaces;
		for(uint i = 0; i < numFaces; ++i) {
			aiFace assimpFace = assimpMesh->mFaces[i];
			uint numIndices = assimpFace.mNumIndices;
			for(uint j = 0; j < numIndices; ++j) {
				indices.push_back(assimpFace.mIndices[j]);
			}
		}
		// 每个 mesh 对应一个材质
		uint materialIndex = assimpMesh->mMaterialIndex;
		if(materialIndex >= 0) {
			aiMaterial* assimpMaterial = assimpScene->mMaterials[materialIndex];
			std::vector<Texture> texesDiffuse = loadMaterialTextures(assimpMaterial, aiTextureType_DIFFUSE, "textureDiffuse");
			textures.insert(textures.end(), texesDiffuse.begin(), texesDiffuse.end());

			std::vector<Texture> texesSpecular = loadMaterialTextures(assimpMaterial, aiTextureType_SPECULAR, "textureSpecular");
			textures.insert(textures.end(),  texesSpecular.begin(), texesSpecular.end());
			// 由于 assimp 对反射纹理的支持性问题, 作者的素材中将反射纹理伪装成了 aiTextureType_AMBIENT
			// 因而下面虽然纹理类型是 aiTextureType_AMBIENT, 但实际加载的是反射纹理
			std::vector<Texture> texesReflection = loadMaterialTextures(assimpMaterial, aiTextureType_AMBIENT, "textureReflection");
			textures.insert(textures.end(),  texesReflection.begin(), texesReflection.end());

			// 还是类似的原因, 作者的模型中的法线纹理不会被 Assimp 的 aiTextureType_NORMAL加载, 但是可以被 aiTextureType_HEIGHT 加载
			// 因而下面的 aiTextureType_HEIGHT 实际加载的是 法线纹理
			std::vector<Texture> texesNormal = loadMaterialTextures(assimpMaterial, aiTextureType_HEIGHT, "textureNormal");
			textures.insert(textures.end(),  texesNormal.begin(), texesNormal.end());
		}
		return Mesh{vertices, indices, textures};
	}

	std::vector<Texture> Model::loadMaterialTextures(aiMaterial* assimpMat, aiTextureType assimpTexType, const std::string& texTypeName) {
		std::vector<Texture> textures;
		uint TexCount = assimpMat->GetTextureCount(assimpTexType);
		
		for(uint i = 0; i < TexCount; ++i) {
			aiString assimpTexFileName;
			assimpMat->GetTexture(assimpTexType, i, &assimpTexFileName); // 获取目标类型的纹理文件的名字
			std::string texFileName = assimpTexFileName.C_Str();
			if(_texturesLoaded.find(texFileName) == _texturesLoaded.end()) {// 没找到
				// 加载
				Texture texture;
				texture._id = TextureFromFile(texFileName);
				texture._typeName = texTypeName;
				texture._fileName = texFileName;
				_texturesLoaded[texFileName] = texture;
				textures.push_back(texture);
			}
			else// 已经被加载过了
				textures.push_back(_texturesLoaded[texFileName]); // 不需要加载, 直接 push
		}

		return textures;
	}

	void Model::LoadMeshBones(std::vector<BoneInfluenceSet>& boneIDweights, aiMesh* assimpMesh, const aiScene* assimpScene) {
		uint numBones = assimpMesh->mNumBones;

		for(uint iBone = 0; iBone < numBones; ++iBone) {
			// 将骨骼id 和 offset 加载到 _bonesLoaded 中
			auto bone = assimpMesh->mBones[iBone];
			std::string boneName = bone->mName.C_Str();
			if(_boneInfoMap.find(boneName) ==_boneInfoMap.end()) { // 没找到
				_boneInfoMap[boneName] = BoneInfo(_boneCount++, AssimpGLMHelpers::ConvertMat4ToGLM_Mat4(bone->mOffsetMatrix));
			}

			// 将骨骼id 和 权重写入到 vetex 中, 每一个骨骼可能会影响多个顶点, 因此一个骨骼要写入到多个顶点中
			// _bonesLoaded[boneName].boneID, 与 vertex.boneIDs[] 中的元素id 和 vertex.weights[] 一一对应
			// 只有第一个mesh 中 iBone 和 boneID 相等, 其它 mesh 的 iBone 会从 0 开始, 但 boneID 会累计变化
			uint boneID = _boneInfoMap[boneName]._id;
			uint numWeights = bone->mNumWeights;
			for(uint iWeight = 0; iWeight < numWeights; ++iWeight) {
				auto vertexWeight = bone->mWeights[iWeight];
				uint vertexID = vertexWeight.mVertexId;
				if(vertexID >= boneIDweights.size()) {
					std::cout << "vertexID >= vertices.size !" << std::endl;
					assert(NULL);
				}
				boneIDweights[vertexID].PushBack(boneID, vertexWeight.mWeight);
			}
		}
	}

	uint Model::TextureFromFile(const std::string& assimpTexFileName/*, const std::string& texTypeName*/) {
		//GLenum target = (texTypeName == "textureReflection" ? GL_TEXTURE_2D : GL_TEXTURE_2D);
		uint textureID;
		glGenTextures(1, &textureID); // 生成纹理对象
		glActiveTexture(GL_TEXTURE0); // 激活纹理单元0, 这里为多个纹理对象使用同一个纹理单元, 在 draw 之前再重新绑定准确的纹理单元
		glBindTexture(GL_TEXTURE_2D, textureID); // 将纹理对象绑定到纹理目标上

		// 设置纹理参数
		// 分别设置 ST 方向的环绕方式
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		// 设置纹理映射缩放算法
		// 当纹理被放大, 线性插值, 避免像素化
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
		// 当纹理被缩小, 使用 MipMap 算法, 且采用三线性插值的算法
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		// 加载纹理图片
		int texWidth, texHeight, nRChannels;
		std::string pathName =  _directory + '/' + assimpTexFileName;
		
		// 是否反转 y 轴应该在外部执行决定
		unsigned char* pImageData = stbi_load(pathName.c_str(), &texWidth, &texHeight, &nRChannels, 0);
		if(!pImageData) 
		{
			std::cerr << "STBI::ERROR::failed to load Image" << '\n';
			exit(1);
		}
		GLenum format;
		if(nRChannels == 4) {format = GL_RGBA;}
		else if(nRChannels == 3) {format = GL_RGB;}
		
		// 生成的纹理图像将保存在 glGenTextures 所生成的对象中
		glTexImage2D(GL_TEXTURE_2D, 0,format, texWidth, texHeight, 0, format, GL_UNSIGNED_BYTE, pImageData);
		glGenerateMipmap(GL_TEXTURE_2D);// 自动生成多级渐远纹理
		stbi_image_free(pImageData);// 释放图片资源

		return textureID;
	}
}

#endif