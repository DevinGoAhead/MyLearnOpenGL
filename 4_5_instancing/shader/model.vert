#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec3 aColor;

out vec3 vColor;
uniform vec2 uOffsets[100];

void main() {
	
	vec2 offset = uOffsets[gl_InstanceID];
	gl_Position = vec4(aPos + offset, 0.f,1.f);
	vColor = aColor;
	// 打印 offset 的值
    // if (gl_InstanceID == 0) {
    //     gl_FragColor = vec4(offset, 0.f, 1.f);
    // }
	//vColor = vec3(uOffsets[gl_InstanceID], 0.f);
}
