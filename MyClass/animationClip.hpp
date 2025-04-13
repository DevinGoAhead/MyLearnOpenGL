#ifndef _ANIMATION
#define _ANIMATION

#include "bone.hpp"
#include "model.hpp"
#include <optional>
#include <functional>

namespace wxy {
	class AnimationClip {
	public:
		AnimationClip(const aiAnimation *assimpAnimation, Model &model);
		float GetDuration();
		int GetTicksPerSecond();
		std::optional<std::reference_wrapper<Bone>> FindBone(const std::string& name);
		const std::map<std::string, BoneInfo>& GetBoneInfoMap()const;
		void PrintBonesInBones() {
			std::cout << "Bones_InBones: "<< std::endl;
			for(const auto& bone : _bones) {std::cout << bone.GetName() << ", ";}
			std::cout << std::endl;
		}
		void PrintBonesInInfoMap() {
			std::cout << "Bones_InfoMap: "<< std::endl;
			for(const auto& bone : _boneInfoMap) {std::cout << bone.first << ", ";}
			std::cout << std::endl;
		}
	private:
		void ReadMissingBones(const aiAnimation *assimpAnimation, Model &model);

	private:
		std::vector<Bone> _bones;
		float _duration; // 当前动画持续时间
		int _ticksPerSecond; // tick, 游戏中的单位时间, 可以与现实世界的单位时间建立映射, 这里明显是与秒建立映射
		std::map<std::string, BoneInfo> _boneInfoMap;
	};

	//public
	AnimationClip::AnimationClip(const aiAnimation *assimpAnimation, Model &model)
	: _duration(assimpAnimation->mDuration), _ticksPerSecond(assimpAnimation->mTicksPerSecond) {
		ReadMissingBones(assimpAnimation, model);
	}

	float AnimationClip::GetDuration() {return _duration;}
	int AnimationClip::GetTicksPerSecond() {return _ticksPerSecond;};
	
	std::optional<std::reference_wrapper<Bone>> AnimationClip::FindBone(const std::string& name) {
		for(auto& bone : _bones) {
    			if(bone.GetName() == name) return std::ref(bone);
		}
		return std::nullopt;
	}
	const std::map<std::string, BoneInfo>& AnimationClip::GetBoneInfoMap()const {return _boneInfoMap;}
	// private
	// Model 中加载的骨骼是依据mesh ,准确的说是mesh 中的顶点, 加载的都是对顶点有影响的骨骼
	// 但是 Animation 中有一些骨骼并未直接影响骨骼, 而是通过影响子骨骼去影响骨骼, 因而 Model 中的骨骼不完整, 这里补充完整
	// 最终的效果是 Animation 中的骨骼在数量和名称上与 Model 中的骨骼一一对应, 这样 transform * localTransform 就可以一一对应了
	// Animation 中存储的变换=是骨骼的层级累计 transform, 而 Model 存储的骨骼是 骨骼空间的局部变换
	// 需要说明的, 后补充的骨骼, 其没有骨骼空间变换, 或者说变换为mat4(1.f)
	void AnimationClip::ReadMissingBones(const aiAnimation *assimpAnimation, Model &model) {
		// model 种的 boneInfoMap 是基于mesh 的, 对于某个模型而言, 是固定的, 与动画无关
		// 当有多个动画时, auto& boneInfoMap 累计补充所有动画的骨骼信息, 这会导致 boneInfoMap 有许多冗余信息
		// 但是, 这样可以减少新的骨骼 ReadMissingBones 的消耗, 使  boneInfoMap 越来越完整
		// 具体取舍, 不确定
		auto& boneInfoMap = model.GetBoneInfoMap();

		uint numChannels = assimpAnimation->mNumChannels; // channel 就是对 Bone 的封装, 类似前面的 class Bone
		for(uint i = 0; i < numChannels; ++i) {
			auto channel = assimpAnimation->mChannels[i];
			std::string boneName = channel->mNodeName.data;
			
			// 对齐 model 和 animation 的骨骼数量, 名称
			if(boneInfoMap.find(boneName) == boneInfoMap.end()){// channel 封装的骨骼, model 中没有
				boneInfoMap[boneName] = BoneInfo(model.GetBoneCount()++); //offset = mat4(1.f)
				std::cout << "Read New Bone: " << boneName << std::endl;
			}
			_bones.emplace_back(boneName, boneInfoMap[boneName]._id, channel); // 依据channel构造 bone, push 到 _bone 中
		}
		_boneInfoMap = boneInfoMap;
	}
}

#endif