#version 410

in vec2 vTexCoord;

out vec3 LightOutput;

uniform vec3 lightDir;
uniform vec3 lightColor;

uniform sampler2D positionTexture;
uniform sampler2D normalTexture;

void main() {
	vec3 normal = normalize(texture(normalTexture, vTexCoord).xyz);
	vec3 position = texture(positionTexture, vTexCoord).xyz;

	float d = max(0, dot(-lightDir, normal));

	LightOutput = lightColor * d;
}