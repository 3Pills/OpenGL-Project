#version 410
in vec4 vNormal;
in vec4 vPos;
out vec4 FragColor;

uniform vec3 AmbientColor;
uniform vec3 MaterialColor;
uniform vec3 LightColor;
uniform vec3 LightDir;
uniform vec3 CameraPos;
uniform float SpecPow;

void main() {
	vec3 N = normalize(vNormal.xyz);
	vec3 L = normalize(LightDir);

	vec3 A = MaterialColor * AmbientColor;

	float d = max(0.0, dot(-L, N));

	vec3 D = vec3(d) * LightColor * MaterialColor;

	vec3 E = normalize(CameraPos - vPos.xyz);
	vec3 R = reflect(L, N);
	float s = max(0, dot(R,E));
	s = pow(s, SpecPow);

	vec3 S = vec3(s) * LightColor * MaterialColor;

	FragColor = vec4(A + D + S,1);
}