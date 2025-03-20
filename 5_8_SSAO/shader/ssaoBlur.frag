#version 330 core

in vec2 vTexCoords;
out vec2 fAlbedoSpec;

uniform sampler2D uTextureAlbedoSpec;

void main() {
	vec2 texSize = 1.f / textureSize(uTextureAlbedoSpec, 0);
	vec2 blurAlbedoSpec = vec2(0.f, 0.f);
	for(int i = -2; i < 3; ++i) {
		for(int j = -2; j < 3; ++j) {
			blurAlbedoSpec += texture(uTextureAlbedoSpec, vTexCoords + texSize * vec2(i, j)).rg;
		}
	}

	fAlbedoSpec = blurAlbedoSpec / 25.f;
}