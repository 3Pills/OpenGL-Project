#version 410

in vec2 vTexCoord;

out vec3 LightOutput;

uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 camPos;

uniform float specPow;

uniform sampler2D positionTexture;
uniform sampler2D normalTexture;

void main() {
	vec3 normal = normalize(texture(normalTexture, vTexCoord).xyz);
	vec3 position = texture(positionTexture, vTexCoord).xyz;
	 
	vec3 N = normalize(normal.xyz);
	vec3 L = normalize(lightDir);

	float d = max(0.0, dot(-L, N));

	vec3 D = vec3(d) * lightColor;

	vec3 E = normalize(camPos - position.xyz);
	vec3 R = reflect(L, N);
	float s = max(0, dot(R,E));
	s = pow(s, specPow);

	vec3 S = vec3(s) * lightColor;

	//LightOutput = D + S;

	LightOutput = D;
}