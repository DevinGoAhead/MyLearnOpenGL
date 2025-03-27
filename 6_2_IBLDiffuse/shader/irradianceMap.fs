#version 330 core

in vec3 vPosWd;
out vec3 fColor;

uniform samplerCube uTextureCube;

void main(){
	vec3 N = normalize(vPosWd); // 世界坐标系
	// 构造一个简单的 TBN, 仅法线对齐
	vec3 T = vec3(1.f, 0.f, 0.f);
	T = normalize(T - N * dot(N, T));
	vec3 B = cross(N, T);
	
	float PI = 3.141592653589;
	float theta = PI / 2.f, phi = 2.f * PI, delta = 0.025;
	int nrSamples = 0;
	vec3 irradiance = vec3(0.f);
	for(float iTheta = 0; iTheta < theta; iTheta += delta) {
		for(float iPhi = 0; iPhi < phi; iPhi += delta) {
			// 局部球坐标系, 朝向一直是向上, 转换到世界坐标系, 朝向与 N相同
			// N 本质是由cube 原点指向立方体的某个面的向量, 由于N 是变化的, 所以这个半球在世界坐标系也是不断旋转的
			// 但是 mat3(T, B, N) 并没有平移部分, 因此这个变换也仅仅是旋转
			// wi 在局部坐标系定义后, 转换到世界坐标系也仅仅是方向变换而已, 原点不重要, 我们可以直接把它放到cube 的原点, samplerCube 采样器要的就是方向
			vec3 wi = vec3(sin(iTheta) * cos(iPhi), sin(iTheta) * sin(iPhi), cos(iTheta)); // 这里是 z 向上的坐标系, 并非 y 向上
			wi = mat3(T, B, N) * wi;

			// 卷积的体现在这里, 本来应该采样到 vPosWd 方向的颜色, 现在采样到了朝向 vPosWd 的方向的半球的周围的平均的颜色
			irradiance += texture(uTextureCube, wi).rgb * cos(iTheta) * sin(iTheta);
			++nrSamples;
		}
	}
	// nrSamples = n1 * n2, 这里的效果类似
	// irradiance += delta * delta * texture(uTextureCube, wi).rgb * cos(iTheta) * sin(iTheta);
	irradiance /= nrSamples; 
	fColor = PI * irradiance;
}