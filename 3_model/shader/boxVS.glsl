#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 _TexCoords_;

uniform mat4 model_;
uniform mat4 view_;
uniform mat4 project_;

void main()
{
    _TexCoords_ = aTexCoords;
    gl_Position = project_ * view_ * model_ * vec4(aPos, 1.0);
	//gl_Position = vec4(aPos, 1.0);
}