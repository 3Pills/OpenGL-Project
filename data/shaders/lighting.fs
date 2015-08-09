#version 410
in vec4 vNormal;
in vec4 vPos;
out vec4 FragColor;

uniform vec3 ambCol;
uniform vec3 matCol;
uniform vec3 lightCol;
uniform vec3 lightDir;
uniform vec3 camPos;
uniform float specPow;

void main() {
	vec3 N = normalize(vNormal.xyz);
	vec3 L = normalize(lightDir);

	vec3 A = matCol * ambCol;

	float d = max(0.0, dot(-L, N));

	vec3 D = vec3(d) * lightCol * matCol;

	vec3 E = normalize(camPos - vPos.xyz);
	vec3 R = reflect(L, N);
	float s = max(0, dot(R,E));
	s = pow(s, specPow);

	vec3 S = vec3(s) * lightCol * matCol;

	FragColor = vec4(A + D + S,1);
}