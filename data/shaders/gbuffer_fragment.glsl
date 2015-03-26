#version 410
in vec4 vPosition;
in vec4 vNormal;

layout(location = 0) out vec3 gPassAlbedo;
layout(location = 1) out vec3 gPassPosition;
layout(location = 2) out vec3 gPassNormal;

out vec3 FragColor;

void main() {
	gPassAlbedo = vec3(1);
	gPassPosition = vPosition.xyz;
	gPassNormal = vNormal.xyz;
}