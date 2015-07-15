#version 410
in vec3 fPosition;
in vec3 fNormal;
in vec3 fTangent;
in vec3 fBiTangent;
in vec2 fTexCoord;

out vec4 fragColor;

uniform vec3 ambCol;
uniform vec3 lightCol;
uniform vec3 lightDir;
uniform vec3 camPos;
uniform float specPow;

uniform sampler2D diffTex;
uniform sampler2D normTex;
uniform sampler2D specTex;

void main() {
	mat3 TBN = mat3(normalize(fTangent), normalize(fBiTangent), normalize(fNormal));

	vec3 sampledNormal = texture(normTex, fTexCoord).xyz;
	vec3 adjustedNormal = sampledNormal * 2 - 1;

	vec3 matCol = texture(diffTex, fTexCoord).xyz;
	vec3 matSpec = texture(specTex, fTexCoord).xyz;

	vec3 N = normalize(TBN * adjustedNormal);
	vec3 L = normalize(lightDir);

	vec3 A = matCol * ambCol;

	float d = max(0.0, dot(-L, N));

	vec3 D = vec3(d) * lightCol * matCol;

	vec3 E = normalize(camPos - fPosition);
	vec3 R = reflect(L, N);
	float s = max(0, dot(R,E));
	s = pow(s, specPow);
	
	vec3 S = vec3(s) * lightCol * matCol;

	fragColor = vec4(A + D + S,1);
}