#version 330 core

in vec3 vPosWd;
out vec4 fColor;

uniform sampler2D uTextureEqRect;

const vec2 invAngle = vec2(0.1591f, 0.3183f); // 1 / 2π, 1 / π

// 本质是要找到 cube 上的点 的方向在等距柱状投影图上对应的坐标
vec2 EqRectToCube(vec3 posCube) {
	// arctan(z / x), 经度线平面 与 x 轴夹角[-π, π]
	// arcsin(y / 1), 纬度线平面 与 OP 的夹角[-π / 2, π / 2].
	vec2 uv = vec2(atan(posCube.z, posCube.x), asin(posCube.y));

	// 转换到[0, 1], 符合纹理坐标规则
	uv *= invAngle; // [-0.5, 0.5]
	return uv += 0.5f;
}

void main(){
	vec3 irrradiance = texture(uTextureEqRect, EqRectToCube(normalize(vPosWd))).rgb; // 坐标必须归一化, 对应到单位球的经纬度
	//irrradiance = vec3(0.8, 0.5, 0.3); // 测试
	fColor = vec4(irrradiance, 1.f);
}