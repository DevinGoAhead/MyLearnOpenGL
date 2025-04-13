#ifndef _BONE
#define _BONE

#include "glm/glm.hpp"
#include <vector>
#include <string>

template<typename T>
struct KeyValue{
	KeyValue(const T& value, float timeStamp) : _value(value), _timeStamp(timeStamp) {}
	T _value;
	float _timeStamp;
};

using KeyPosition = KeyValue<glm::vec3>;
using KeyScale = KeyValue<glm::vec3>;
using KeyRotation = KeyValue<glm::quat>;

namespace wxy{
	// 从 aiAnimation::Channels[] 中提取骨骼空间变换
	class Bone {
	public:
		Bone() = default;
		Bone(const std::string& boneName, int boneID, const aiNodeAnim* channel);
		void UpdateTransform(float curAnimaTime);
		const std::string& GetName()const;
		int GetID();
		const glm::mat4& GetLocalTransform()const;
	private:
		glm::mat4 InterpolateKeyPosition(float curAnimaTime);
		glm::mat4 InterpolateKeyScale(float curAnimaTime);
		glm::mat4 InterpolateKeyRotation(float curAnimaTime);
		// 这里尝试了模板函数, 写起来很繁琐, 总共需要判断9次类型
		// template<typename T>
		// glm::mat4 InterpolateKeyFrame(const std::vector<T> &keyRotations, float curAnimaTime);
	private:
		std::string _name;
		int _ID;
		glm::mat4 _localTransform; // 在骨骼空间的变换
		std::vector<KeyPosition> _keyPositions;
		std::vector<KeyScale> _keyScales;
		std::vector<KeyRotation> _keyRotations;
		
	};

	//public
	Bone::Bone(const std::string& boneName, int boneID, const aiNodeAnim* channel)
		: _name(boneName), _ID(boneID), _localTransform(glm::mat4(1.f)) {
		uint numPositionKeys = channel->mNumPositionKeys;
		for(uint i = 0; i <numPositionKeys; ++i) {
			_keyPositions.emplace_back(AssimpGLMHelpers::ConvertVec3ToGLM_Vec3(channel->mPositionKeys[i].mValue),
									channel->mPositionKeys[i].mTime);
		}

		uint numScalingKeys = channel->mNumScalingKeys;
		for(uint i = 0; i < numScalingKeys; ++i) {
			_keyScales.emplace_back(AssimpGLMHelpers::ConvertVec3ToGLM_Vec3(channel->mScalingKeys[i].mValue),
									channel->mScalingKeys[i].mTime);
		}

		uint numRotationKeys = channel->mNumRotationKeys;
		for(uint i = 0; i < numRotationKeys; ++i) {
			_keyRotations.emplace_back(AssimpGLMHelpers::ConvertQuatToGLM_Quat(channel->mRotationKeys[i].mValue),
									channel->mRotationKeys[i].mTime);
		}
	}

	void Bone::UpdateTransform(float curAnimaTime) {
		_localTransform = InterpolateKeyPosition(curAnimaTime)
						* InterpolateKeyRotation(curAnimaTime)
						* InterpolateKeyScale(curAnimaTime);

		// _localTransform = InterpolateKeyScale(curAnimaTime)
		// 				* InterpolateKeyRotation(curAnimaTime)
		// 				* InterpolateKeyPosition(curAnimaTime);
	}

	const std::string& Bone::GetName()const {return _name;}
	int Bone::GetID() {return _ID;}
	const glm::mat4& Bone::GetLocalTransform()const {return _localTransform;}

	//private
	glm::mat4 Bone::InterpolateKeyPosition(float curAnimaTime) {
		if(_keyPositions.empty()) return glm::mat4(1.f);
		if(_keyPositions.size() == 1) return glm::translate(glm::mat4(1.f), _keyPositions.front()._value); // 只有 1 个关键帧

		for(auto iter = _keyPositions.begin() + 1; iter != _keyPositions.end(); ++iter) {
			if(curAnimaTime < iter->_timeStamp) {
				auto last = iter - 1;
				auto next = iter;
				float t = (curAnimaTime - last->_timeStamp) / (next->_timeStamp - last->_timeStamp);
				glm::vec3 interPosition = glm::mix(last->_value, next->_value, t);
				return glm::translate(glm::mat4(1.f), interPosition);
			}
		}
		// 说明到了最后的时间
		return glm::translate(glm::mat4(1.f), _keyPositions.back()._value);
	}

	glm::mat4 Bone::InterpolateKeyScale(float curAnimaTime) {
		if(_keyScales.empty()) return glm::mat4(1.f);
		if(_keyScales.size() == 1) return glm::scale(glm::mat4(1.f), _keyScales.front()._value); // 只有 1 个关键帧

		for(auto iter = _keyScales.begin() + 1; iter != _keyScales.end(); ++iter) {
			if(curAnimaTime < iter->_timeStamp) {
				auto last = iter - 1;
				auto next = iter;
				float t = (curAnimaTime - last->_timeStamp) / (next->_timeStamp - last->_timeStamp);
				glm::vec3 interScale = glm::mix(last->_value, next->_value, t);
				return glm::scale(glm::mat4(1.f), interScale);
			}
		}
		// 说明到了最后的时间
		return glm::scale(glm::mat4(1.f), _keyScales.back()._value);
	}

	glm::mat4 Bone::InterpolateKeyRotation(float curAnimaTime) {
		if(_keyRotations.empty()) return glm::mat4(1.f);
		if(_keyRotations.size() == 1) return glm::mat4_cast(_keyRotations.front()._value); // 只有 1 个关键帧

		for(auto iter = _keyRotations.begin() + 1; iter != _keyRotations.end(); ++iter) {
			if(curAnimaTime < iter->_timeStamp) {
				auto last = iter - 1;
				auto next = iter;
				float t = (curAnimaTime - last->_timeStamp) / (next->_timeStamp - last->_timeStamp);
				glm::quat interRotation = glm::slerp(last->_value, next->_value, t);
				return glm::mat4_cast(interRotation);
			}
		}
		// 说明到了最后的时间
		return glm::mat4_cast(_keyRotations.back()._value);
	}
}

#endif