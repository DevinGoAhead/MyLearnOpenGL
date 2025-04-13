#ifndef _SKELETON
#define _SKELETON

#include <string>

struct HierarchyNode {
	std::string _boneName;
	glm::mat4 _transform; // 层级累计变换
	std::vector<HierarchyNode> _children;
};

namespace wxy {
	class Skeleton {
		using Node = HierarchyNode;

	public:
		Skeleton(const std::string& animaPathName);
		Node& GetRootNode();

		void PrintBonesInHierarchy() {
			std::cout << "Bones_Hierarchy: "<< std::endl;
			_PrintBonesInHierarchy(_rootNode);
			std::cout << std::endl;
		}
	private:
		void ReadHierarchyData(Node& dest, const aiNode *src);
		void _PrintBonesInHierarchy(const Node& node) {
			for(const auto& _node : node._children) {
				_PrintBonesInHierarchy(_node);
			}
			std::cout << node._boneName << ", ";
		}
	private:
		Node _rootNode;
	};

	//public
	Skeleton::Skeleton(const std::string& animaPathName) {
		Assimp::Importer importer;
		const aiScene* assimpScene = importer.ReadFile(animaPathName, aiProcess_Triangulate);
		// 只要有一个为假即判断为失败
		if(!assimpScene || (assimpScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !assimpScene->mRootNode)) {
			std::cerr << "ASSIMP::ERROR::" << importer.GetErrorString() << std::endl;
		}
		ReadHierarchyData(_rootNode, assimpScene->mRootNode); // 读取层级累计变换结构
	}

	typename Skeleton::Node& Skeleton::GetRootNode() {return _rootNode;}

	//private
	void Skeleton::ReadHierarchyData(Node& dest, const aiNode *src) {
		// 第一次迭代写入的是外部传入的 dest
		// 新的迭代写入的是上一次新建的 newNode
		dest._boneName = src->mName.data;
		dest._transform = AssimpGLMHelpers::ConvertMat4ToGLM_Mat4(src->mTransformation);
		// 当无子节点时, 不会进入for 循环, 停止迭代
		for(uint i = 0; i < src->mNumChildren; ++i) {
			// 新建的节点在下一次迭代中写入
			// 但需要等待子节点的循环结束后才能push, 本质是一个类似后序的逻辑
			Node newNode; 
			ReadHierarchyData(newNode, src->mChildren[i]);
			dest._children.push_back(newNode);
		}
	}
}

#endif