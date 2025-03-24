#version 330 core
layout (location = 0) out vec3 gBufPosition; // 位置, COLOR_ATTACHMENT0
layout (location = 1) out vec3 gBufNormal; // 法线, COLOR_ATTACHMENT1
layout (location = 2) out vec2 gBufAlbedoSpec; // albedo + spec intensity, COLOR_ATTACHMENT2

struct Material {
	sampler2D textureSpecular0;
};

in VOUT {
	vec3 pos;
	vec3 normal;
	vec2 texCoords;
} fIn;

uniform Material uMaterial;

void main() {
	gBufPosition = fIn.pos;
	vec3 normal = fIn.normal;
	if(!gl_FrontFacing) normal = -normal;
	gBufNormal = normalize(normal);
	gBufAlbedoSpec.r = 0.95f;// 自定义了一个颜色
	gBufAlbedoSpec.g = texture(uMaterial.textureSpecular0, fIn.texCoords).r;
}