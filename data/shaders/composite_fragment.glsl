#version 410

in vec2 vTexCoord;

out vec4 FragColour;

uniform sampler2D albedoTexture;
uniform sampler2D lightTexture;

uniform bool drawLight;
uniform bool drawAlbedo;

void main() {
	vec4 light = texture(lightTexture, vTexCoord);
	vec4 albedo = texture(albedoTexture, vTexCoord);
	FragColour = albedo * light;
}