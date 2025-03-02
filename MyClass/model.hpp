#ifndef __MODEL
#define __MODEL

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h" // 加载纹理图像

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "mesh.hpp"
#include <iostream>
#include <map>
#include <cstring>

namespace wxy {
	class Model {
	public:
		Model(const std::string& pathName);
		void Draw(wxy::ShaderProgram shaderProgram);
		//~Model();
	private:
		void LoadModel(const std::string& pathName); // 加载模型, pathName 模型文件的路径
		void ProcessNode(aiNode* assimpNode, const aiScene* assimpScene); // 遍历所有节点 及其 子节点
		Mesh ProcessMesh(aiMesh* assimpMesh, const aiScene* assimpScene);
		std::vector<Texture> loadMaterialTextures(aiMaterial* assimpMat, aiTextureType assimpTexType, const std::string& texTypeName);
		uint TextureFromFile(const std::string& assimpTexFileName/*, const std::string& texTypeName*/);
	private:
		std::vector<Mesh> _meshes; // 存放加载的所有mesh
		std::map<std::string, Texture> _texturesLoaded; // 已经加载过的模型
		std::string _directory; // 不含文件名的模型文件的路径
	};

	//public
	Model::Model(const std::string& pathName) {
		LoadModel(pathName);
	}
	void Model::Draw(ShaderProgram shaderProgram) {
		uint size = _meshes.size();
		//std::cout << size << std::endl;
		for(uint i = 0; i <  size; ++i) {
			_meshes[i].Draw(shaderProgram);
		}
	}
	//private
	void Model::LoadModel(const std::string& pathName) {
		Assimp::Importer import;
		const aiScene *assimpScene = import.ReadFile(pathName, 
				aiProcess_Triangulate | aiProcess_FlipUVs); // 将非三角形处理为三角形 | 反转纹理图像的 y 坐标
		// 只要有一个为假即判断为失败
		if(!assimpScene || (assimpScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !assimpScene->mRootNode)) {
			std::cerr << "ASSIMP::ERROR::" << import.GetErrorString() << std::endl;
		}
		
		_directory = pathName.substr(0, pathName.find_last_of('/')); // 使用 [0, 最后一个/) 之间的内容创建一个子字符串, 其实就是不含文件名的路径
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

			vertices.push_back(vertex);
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