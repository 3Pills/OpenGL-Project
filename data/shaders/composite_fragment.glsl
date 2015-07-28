#version 410

in vec2 vTexCoord;

out vec4 FragColour;

uniform sampler2D albedoTexture; //Base geometry colour info.
uniform sampler2D lightTexture; //Light color modifications to geometry.
uniform sampler2D fxTexture; //effects (particles) colour info.

uniform bool drawLight;
uniform bool drawAlbedo;

void main() {
	vec4 albedo = texture(albedoTexture, vTexCoord);
	vec4 light = texture(lightTexture, vTexCoord);
	vec4 fx = texture(fxTexture, vTexCoord);

	//fx objects are rendered independant of the other two, so they blend additively.
	FragColour = albedo * light + fx;
}