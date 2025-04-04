#ifndef __MATERIAL
#define __MATERIAL

#include <string>
#include <unordered_map>
#include "glad/glad.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h" // 加载纹理图像
#endif


namespace wxy{
	enum TextureType{
		Albedo, Normal, Metallicity, Roughness, AO
	};
	std::vector<std::string> texNames{"Albedo", "Normal", "Metallicity", "Roughness", "AO"};
	struct MTexture{
		MTexture(uint id, const std::string& name) : _texID(id), _texName(name) {}
		uint _texID;
		std::string _texName;
	};

	class Material{
	public:
		Material(){}

		void PushBackTexture(TextureType textureType, std::string pathName, 
			bool isGammaCorrect = false, bool isFlip = false, GLenum target = GL_TEXTURE_2D, GLint wrappingParam = GL_REPEAT, 
			GLint magFilterParam = GL_LINEAR, GLint minFilterParam = GL_NEAREST) {
			uint tex;
			SetTexParameter(tex, target, magFilterParam, minFilterParam, wrappingParam);
			GenerateTex(target, pathName.c_str(), isGammaCorrect, isFlip);

			_textures.emplace(textureType, MTexture(tex, texNames[textureType]));
		}
		//uint& GetTextureID(TextureType textureType) {return _textureIDs[textureType];}
		void Bind(ShaderProgram& shaderProgram, int texChannelOffset, GLenum target = GL_TEXTURE_2D) {
			shaderProgram.Use();
			int i = 1;
			for(const auto& tex : _textures) {
				glActiveTexture(GL_TEXTURE0 + texChannelOffset + i);
				glBindTexture(target, tex.second._texID);
				#ifdef DEBUG
				std::cout << "Material UniformName: " << "uTexture" + tex.second._texName << std::endl;
				#endif
				shaderProgram.SetUniform(("uTexture" + tex.second._texName).c_str(), texChannelOffset + i++);
			}
		}
	private:
		void SetTexParameter(GLuint& tex, GLenum target, GLint wrappingParam, GLint magFilterParam, GLint minFilterParam){
			glGenTextures(1, &tex);
			glBindTexture(target, tex);
			// 设置纹理环绕参数
			glTexParameteri(target, GL_TEXTURE_WRAP_S, wrappingParam);
			glTexParameteri(target, GL_TEXTURE_WRAP_T, wrappingParam);
			if(target == GL_TEXTURE_CUBE_MAP) glTexParameteri(target, GL_TEXTURE_WRAP_R, wrappingParam);
			
			// 这里暂时先这样, 后续再优化
			if(wrappingParam == GL_CLAMP_TO_BORDER) {
				GLfloat borderColor[] ={1.0f, 1.0f, 1.0f, 1.0f};
				glTexParameterfv(target, GL_TEXTURE_BORDER_COLOR, borderColor);
			}
			// 设置纹理映射(过滤)方式
			glTexParameteri(target, GL_TEXTURE_MAG_FILTER, magFilterParam); // 放大
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, minFilterParam); // 缩小
		};

		void GenerateTex(GLenum target, const char* pathName, bool isGammaCorrect, bool isFlip){
			stbi_set_flip_vertically_on_load(isFlip); // 是否翻转
			// 加载纹理
			int imageWidth, imageHeight, nChannels;
			unsigned char* pImageData = stbi_load(pathName, &imageWidth, &imageHeight, &nChannels, 0);
			if(!pImageData) {
				std::cerr << "failed to load Image" << '\n';
				exit(1);
			}
	
			GLint internalFormat;
			GLint format;
			// 这里暂时先这样, 后续再优化
			if(nChannels == 4) {
				internalFormat = isGammaCorrect ?  GL_SRGB_ALPHA : GL_RGBA;
				format = GL_RGBA;
			}
			else if(nChannels == 3){
				internalFormat = isGammaCorrect ?  GL_SRGB : GL_RGB;
				format = GL_RGB;
			}
			else if(nChannels == 1){
				internalFormat = GL_RED;
				format = GL_RED;
			}
			// 开辟内存, 存储图片
			glTexImage2D(target, 0, internalFormat, imageWidth, imageHeight, 0, format, GL_UNSIGNED_BYTE, pImageData);
			glGenerateMipmap(target); // 添加Mipmap生成
			stbi_image_free(pImageData);
		};
	private:
		//std::string _name;
		std::unordered_map<TextureType, MTexture> _textures;
	};
}

#endif