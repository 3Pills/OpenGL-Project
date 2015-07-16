#version 410

in vec3 fPosition;
in vec3 fNormal;

layout(location = 0) out vec4 gPassAlbedo;
layout(location = 1) out vec3 gPassPosition;
layout(location = 2) out vec3 gPassNormal;

void main() {
	gPassAlbedo = vec4(1);
	gPassPosition = fPosition;
	gPassNormal = fNormal;
}