#ifndef CAMERA
#define CAMERA

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

namespace wxy
{
	const float Pitch = 0.f;
	const float Yaw = -90.f;
	const float Fov = 45.f;
	const float Sensitivity = 0.05f;
	const float Speed = 8.f;
	const glm::vec3 WorldUp = {0.f, 1.f, 0.f};
	enum MouseMoveDir
	{
		Front, Back, Left, Right
	};

	class Camera
	{
	public:

		Camera(glm::vec3 pos = {0.f, 0.f, 0.f}, glm::vec3 front = {0.f, 0.f, -1.f}, glm::vec3 up = WorldUp,
			float pitch = Pitch, float yaw = Yaw,
			float fov = Fov)
			: _pos(pos), _front(front), _up(up), _right(glm::normalize(glm::cross(_front, _up)))
			, _pitch(pitch), _yaw(yaw)
			, _fov(fov)
		{}
		void ProcessMouseMove(float dx, float dy, bool pitchConstrain = true)
		{
			// 更新_pitch, _yaw
			dx *= Sensitivity, dy *= Sensitivity;
			_yaw += dx, _pitch += dy;

			if(pitchConstrain)
			{
				if(_pitch > 89.f) _pitch = 89.f;
				if(_pitch < -89.f) _pitch = -89.f;
			}
			
			updateCameraVectors();// 更新 camera
		}

		void ProcessMouseScroll(float dx, float dy)
		{
			_fov += dy;
			if(_fov > 45.f) _fov = 45.f;
			if(_fov < 0.1f) _fov = 0.1f;
		}

		void ProcessKeyBoard(MouseMoveDir mouseDir, float perFrameTime)
		{
			float speed = 8 * perFrameTime;
			switch (mouseDir)
			{
				case Front : // 前
				{
					_pos += (speed * _front);
					break;
				}
				case Back : // 后
				{
					_pos -= (speed * _front);
					break;
				}
				case Left : // 左
				{
					_pos -= (speed * _right);
					break;
				}
				case Right : // 右
				{
					_pos += (speed * _right);
					break;
				}
				default:
				{

				}
			}
		}

		glm::mat4 GetViewMatrix() {return glm::lookAt(_pos, _front + _pos, _up);}
		float GetFov() {return _fov;}
		glm::vec3 GetPos() {return _pos;}
		glm::vec3 GetFront() {return _front;}
	private:
	    void updateCameraVectors()
		{
			_front.y = glm::sin(glm::radians(_pitch));
			
			float rXZ = glm::cos(glm::radians(_pitch));
			_front.x = rXZ * glm::cos(glm::radians(_yaw));
			_front.z = rXZ * glm::sin(glm::radians(_yaw));
			
			_front = glm::normalize(_front);

			_right = glm::normalize(glm::cross(_front, WorldUp));
			_up = glm::normalize(glm::cross(_right, _front));
		}
	private:
		glm::vec3 _pos;
		glm::vec3 _front;
		glm::vec3 _up;
		glm::vec3 _right;
		float _pitch; // 与 y 轴夹角
		float _yaw; // 与 x 轴夹角
		float _fov; // 竖向可视角度
	};
}

#endif