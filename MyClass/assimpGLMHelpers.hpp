#ifndef __ASSIMPGLMHELPERS
#define __ASSIMPGLMHELPERS

#include<assimp/quaternion.h>
#include<assimp/vector3.h>
#include<assimp/matrix4x4.h>

#include<glm/glm.hpp>
#include<glm/gtc/quaternion.hpp>
namespace wxy{
	class AssimpGLMHelpers {
	public:
		// static 修饰, 成员函数属于类, 不需要创建类对象即可调用
		static inline glm::mat4 ConvertMat4ToGLM_Mat4(const aiMatrix4x4& from) {
			// return {from.a1, from.a2, from.a3, from.a4, // 这一行会作为 glm::mat4 的第一列, 下同理
			// 		from.b1, from.b2, from.b3, from.b4,
			// 		from.c1, from.c2, from.c3, from.c4,
			// 		from.d1, from.d2, from.d3, from.d4};

			glm::mat4 to;
			//the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
			to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
			to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
			to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
			to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
			return to;
		}

		static inline glm::vec3 ConvertVec3ToGLM_Vec3(const aiVector3D& from) {
			return {from.x, from.y, from.z};
		}

		static inline glm::quat ConvertQuatToGLM_Quat(const aiQuaternion& from) {
			return {from.w, from.x, from.y, from.z}; // w 实部
		}
	};
}
#endif