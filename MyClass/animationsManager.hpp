#ifndef _ANIMATIONSMANAGER
#define _ANIMATIONSMANAGER

#include "animationClip.hpp"
#include <memory>

namespace wxy {
	class AnimationsManager {
	public:
		AnimationsManager() = default;
		void LoadAnimations(const std::string& animaPathName, Model& model);
		void LoadAnimations(const std::string& animaPathName, const std::string& animationName, Model& model);
		std::shared_ptr<AnimationClip> GetAnimation(const std::string& name);
		void PrintAnimationsName() {
			std::cout << "AnimationsName: "<< std::endl;
			for(const auto& animation : _animationMap) { std::cout << animation.first << ", ";}
			std::cout << std::endl;
		}
	private:
		std::map<std::string, std::shared_ptr<AnimationClip>> _animationMap;
	};

	//public
	void AnimationsManager::LoadAnimations(const std::string& animaPathName, Model& model) {
		Assimp::Importer importer;
		const aiScene* assimpScene = importer.ReadFile(animaPathName, aiProcess_Triangulate);
		// 只要有一个为假即判断为失败
		if(!assimpScene || (assimpScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !assimpScene->mRootNode)) {
			std::cerr << "ASSIMP::ERROR::" << importer.GetErrorString() << std::endl;
		}
		auto animations = assimpScene->mAnimations;
		uint numAnimations = assimpScene->mNumAnimations;
		for(uint i = 0; i < numAnimations; ++i) {
			_animationMap[animations[i]->mName.data] = std::make_shared<AnimationClip>(AnimationClip(animations[i], model));
		}
	}

	// 针对单文件存储单个动画, 且需要自定义动画名称时
	void AnimationsManager::LoadAnimations(const std::string& animaPathName, const std::string& animationName, Model& model) {
		Assimp::Importer importer;
		const aiScene* assimpScene = importer.ReadFile(animaPathName, aiProcess_Triangulate);
		// 只要有一个为假即判断为失败
		//  if(!assimpScene || (assimpScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !assimpScene->mRootNode))
		// 可能是assimp 加载的问题, 动画可以播放, 但却总是报错
		// 这里移除对 | (assimpScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !assimpScene->mRootNode) 的检查
		if(!assimpScene) {
			std::cerr << "ASSIMP::ERROR::" << importer.GetErrorString() << std::endl;
		}

		auto animations = assimpScene->mAnimations;
		
		_animationMap[animationName] = std::make_shared<AnimationClip>(AnimationClip(animations[0], model));
	}
	std::shared_ptr<AnimationClip> AnimationsManager::GetAnimation(const std::string& name) {return _animationMap.find(name)->second;}

}

#endif