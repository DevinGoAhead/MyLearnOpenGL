#ifndef _ANIMATOR
#define _ANIMATOR

#include "skeleton.hpp"
#include "animationClip.hpp"
#include <memory>

namespace wxy {
	const int MaxBones = 100;
	class Animator {
	using Node = HierarchyNode;
	public:
		Animator(const Skeleton& skeleton);
		void Play(std::shared_ptr<AnimationClip> newAnimation);
		void UpdateAnimation(float dTime);
		const std::vector<glm::mat4>& GetFinalTransforms()const;
	private:
		void CalculateTransform(Node& node, glm::mat4 parentTransform);
	private:
		Skeleton _skeleton;
		std::shared_ptr<AnimationClip> _currentAnimation;
		float _currentTime;
		//float _dTime; // 也没用啊?
		std::vector<glm::mat4> _finalTransforms;
	};

	//public
	Animator::Animator(const Skeleton& skeleton)
	: _skeleton(skeleton), _currentTime(0.f) {
	}

	void Animator::Play(std::shared_ptr<AnimationClip> newAnimation) {
		if(_currentAnimation == newAnimation) return;
		_currentAnimation = newAnimation;
		_finalTransforms.resize(MaxBones);
		std::fill(_finalTransforms.begin(), _finalTransforms.end(), glm::mat4(1.f));
		_currentTime = 0.f;
	}
	void Animator::UpdateAnimation(float dTime) {
		if(!_currentAnimation) {
			std::cout << "Invalid animation !" << std::endl;
			return;
		};
		_currentTime +=  _currentAnimation->GetTicksPerSecond() * dTime; // 换算为动画世界的时间
		_currentTime = std::fmod(_currentTime, _currentAnimation->GetDuration()); // 浮点数求模, 确保 _currentTime <= duration
		CalculateTransform(_skeleton.GetRootNode(), glm::mat4(1.f));
	}

	const std::vector<glm::mat4>& Animator::GetFinalTransforms()const {return _finalTransforms;}

	//private
	void Animator::CalculateTransform(Node& node, glm::mat4 parentLocalTransform) {
		glm::mat4 nodeLocalTransform = node._transform;
		std::string boneName = node._boneName;
		auto bone = _currentAnimation->FindBone(boneName);

		//localTransform
		if(bone) {
			bone->get().UpdateTransform(_currentTime);
			nodeLocalTransform = bone->get().GetLocalTransform();
		}
		else {
			//std::cout << boneName << " was not found in animation bones!" << std::endl;
			//nodeLocalTransform = glm::mat4(1.f)
		}
		// 我最初的命名是 heirarchyLocalTransform
		// 这个变换是将骨骼空间中的点经过一系列变换得到点基于模型空间原点的新位置
		glm::mat4 heirarchyGlobalTransform = parentLocalTransform * nodeLocalTransform;

		//offset
		auto boneInfoMap =  _currentAnimation->GetBoneInfoMap();
		auto iter = boneInfoMap.find(boneName);

		if(iter != boneInfoMap.end()){
			//std::cout << boneName << " was found in mesh bones!" << std::endl;
			// 先将点从模型空间转换到骨骼空间, 再基于骨骼空间做一系列的变换, 变换的终点是回到模型空间
			glm::mat4 finalTransform = heirarchyGlobalTransform * iter->second._offset;
			_finalTransforms[iter->second._id] = finalTransform; // finalTransform 与骨骼一一对应
			//std::cout << boneName << ": " << glm::to_string(_finalTransforms[iter->second._id]) << std::endl;
		}
		else {
			//std::cout << boneName << " was not found in mesh bones!" << std::endl;
		}

		for(auto& node : node._children) {
			CalculateTransform(node, heirarchyGlobalTransform);
		}
	}
}

#endif