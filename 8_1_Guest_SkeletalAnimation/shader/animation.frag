#version 330
struct Material {
	sampler2D textureDiffuse;
};

in vec2 vTexCoord;
out vec4 fColor;

uniform Material uMaterial;

void main() {
	fColor = vec4(texture(uMaterial.textureDiffuse, vTexCoord).rgb, 1.f);
	//fColor = vec4(vec3(0.7f), 1.f);
}