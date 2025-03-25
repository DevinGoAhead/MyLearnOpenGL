#ifndef __SPHERE
#define __SPHERE

#include <vector>
#include <cmath>
#include <numbers>
#include "glm/glm.hpp"

namespace wxy {
	using uint = unsigned int;

	struct Triangle{
		uint _aIndex;
		uint _bIndex;
		uint _cIndex;
	}; // 逆时针

	struct Vertex_{
		glm::vec3 _position;
		glm::vec3 _normal;
		glm::vec2 _texCoord;
	};

	class Sphere{
	public:
	    // 将球划分出 latitudeBands 个纬度(其实得到了 LatitudeBands + 1 条纬度线)
	    // 将球划分出 longitudeBands 个经度(其实得到了 LongitudeBands + 1 条纬度线,且第一条和最后一条是重合的)
	    // 顶点总数其实是 (latitudeBands +1) * (longitudeBands + 1)个
		Sphere(const uint latitudeBands = 64, const uint longitudeBands = 64, float radius = 1.f);
		void Draw();
	private:
		void SetupMesh();
	private:
		std::vector<Vertex_> _vertices;
		std::vector<Triangle> _indices;
		uint _VBO, _EBO, _VAO;
		std::vector<uint> _textureIDs;
		float _r; // radius
	};

	Sphere::Sphere(const uint latitudeBands, const uint longitudeBands, float radius) : _r(radius){
		_vertices.reserve((latitudeBands + 1) * (longitudeBands + 1));

		for(uint iLat = 0; iLat <= latitudeBands; ++iLat) {
			float phi = ((float)iLat / latitudeBands) * std::numbers::pi; // OP 与 y 轴的夹角, 范围为 [0, π]
			for(uint iLongi = 0; iLongi <= longitudeBands; ++iLongi) {
				float theta = ((float)iLongi / longitudeBands) * 2.f * std::numbers::pi; // OP 在 xOz 面投影线 ON 与 x 轴的夹角,范围为 [0, 2π]
				
				Vertex_ vertex;
				glm::vec3 temp;

				float cosPhi = cos(phi), sinPhi = sin(phi), cosTheta = cos(theta), sinTheta = sin(theta);
				
				temp.x = sinPhi * cosTheta, temp.y = cosPhi, temp.z = -sinPhi * sinTheta;
				vertex._normal = temp;
				
				temp *= _r;
				vertex._position = temp;

				vertex._texCoord ={(float)iLat / latitudeBands, (float)iLongi / longitudeBands};

				_vertices.push_back(vertex);
				// 创建顶点索引数据
				// 每个基准点扩充出 2 个三角形, 忽略最后一行, 最后一列(行列是指如果能将球展开的情况下)
				Triangle triangle;
				if(iLat < latitudeBands && iLongi < longitudeBands)
				{
					triangle._aIndex = iLat * (longitudeBands + 1) + iLongi; //每一行点的个数是 longitudeBands + 1
					triangle._bIndex = (iLat + 1) * (longitudeBands + 1) + iLongi;
					triangle._cIndex = (iLat + 1) * (longitudeBands + 1) + (iLongi + 1);
					_indices.push_back(triangle);

					//第二个三角形与第一个三角形共线 ac, 因此仅计算另一个点即可
					triangle._bIndex = triangle._aIndex + 1;

					std::swap(triangle._bIndex, triangle._cIndex);//确保逆时针定义三角形
					_indices.push_back(triangle);
				}
			}
		}
		SetupMesh();
	}

	void Sphere::Draw() {
		// draw
		glBindVertexArray(_VAO);
		glDrawElements(GL_TRIANGLES, 3 * _indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0); // 解绑
	}

	//private
	void Sphere::SetupMesh() {
		glGenBuffers(1, &_VBO); // 创建一个 buffer, 绑定 _VBO, 将来将作为 vertex buffer
		glGenBuffers(1, &_EBO); // 创建一个 buffer, 绑定 _EBO, 将来将作为 element(index) buffer
		glGenVertexArrays(1, &_VAO); // 创建一个 vertex array, 绑定 _VAO

		glBindVertexArray(_VAO); // 开始记录
		glBindBuffer(GL_ARRAY_BUFFER, _VBO); // 告知 GPU, _VBO 绑定的是 vertex buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _EBO); // 告知 GPU, _EBO 绑定的是 element(index) buffer
		
		glBufferData(GL_ARRAY_BUFFER, _vertices.size() * sizeof(Vertex_), &_vertices[0], GL_STATIC_DRAW); // GPU 中开辟内存存储 _vertices
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, _indices.size() * sizeof(Triangle), _indices.data(), GL_STATIC_DRAW);// GPU 中开辟内存存储 _indices

		// 告知 GPU 如何解析顶点数据
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_), (void*)offsetof(Vertex_, _position)); // 位置坐标
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_), (void*)offsetof(Vertex_, _normal)); // 法线坐标
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex_), (void*)offsetof(Vertex_, _texCoord)); // 纹理坐标
		glEnableVertexAttribArray(0), glEnableVertexAttribArray(1), glEnableVertexAttribArray(2);

		glBindVertexArray(0);// 解绑 _VAO, 停止记录
	}
}
#endif