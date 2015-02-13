#version 410
in vec3 fPos;
in vec3 fNormal;
in vec3 fTangent;
in vec3 fBiTangent;
in vec2 fTexCoord;

out vec4 FragColor;

uniform vec3 ambientColor;
uniform vec3 lightColor;
uniform vec3 lightDir;
uniform vec3 cameraPos;
uniform float specPow;

uniform sampler2D diffTex;
uniform sampler2D normTex;
uniform sampler2D specTex;

void main() {
	mat3 TBN = mat3(normalize(fTangent), normalize(fBiTangent), normalize(fNormal));

	vec3 sampledNormal = texture(normTex, fTexCoord).xyz;
	vec3 adjustedNormal = sampledNormal * 2 - 1;

	vec3 N = normalize(TBN * adjustedNormal);

	vec3 materialColor = texture(diffTex, fTexCoord).xyz;
	vec3 materialSpec = texture(specTex, fTexCoord).xyz;

	vec3 L = normalize(lightDir);

	vec3 A = materialColor * ambientColor;

	float d = max(0.0, dot(-L, N));

	vec3 E = normalize(cameraPos - fPos);
	vec3 R = reflect(L, N);
	float s = max(0, dot(R,E));
	s = pow(s, specPow);
	

	vec3 D = vec3(d) * lightColor * materialColor;
	vec3 S = vec3(s) * lightColor * materialSpec;

	FragColor = vec4(A + D + S,1);
}