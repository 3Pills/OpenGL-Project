#version 410

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoord;

layout(location = 0) out vec3 gPassAlbedo;
layout(location = 1) out vec3 gPassPosition;
layout(location = 2) out vec3 gPassNormal;

uniform sampler2D diffuse;
uniform bool textured;

void main() {
	gPassAlbedo = vec3(1);
	if (textured) {
		gPassAlbedo = texture(diffuse, fTexCoord).xyz;
	}
	gPassPosition = fPosition;
	gPassNormal = fNormal;
}