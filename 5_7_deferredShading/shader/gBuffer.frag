#version 330 core
layout (location = 0) out vec3 gBufPosition; // 位置, COLOR_ATTACHMENT0
layout (location = 1) out vec3 gBufNormal; // 法线, COLOR_ATTACHMENT1
layout (location = 2) out vec4 gBufAlbedoSpec; // albedo + spec intensity, COLOR_ATTACHMENT2

struct Material {
	sampler2D textureDiffuse0;
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
	gBufNormal = normalize(fIn.normal);
	gBufAlbedoSpec.rgb = texture(uMaterial.textureDiffuse0, fIn.texCoords).rgb;
	gBufAlbedoSpec.a = texture(uMaterial.textureSpecular0, fIn.texCoords).r;
}