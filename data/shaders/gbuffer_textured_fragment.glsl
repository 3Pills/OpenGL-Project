#version 410

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoord;
in vec4 fColor;

layout(location = 0) out vec4 gPassAlbedo;
layout(location = 1) out vec4 gPassPosition;
layout(location = 2) out vec4 gPassNormal;

uniform sampler2D diffuse;
uniform sampler2D normal;
uniform sampler2D specular;

void main() {
	gPassAlbedo = texture(diffuse, fTexCoord) * fColor;
	gPassPosition = vec4(fPosition.xyz, gPassAlbedo.a);
	gPassNormal = vec4(fNormal.xyz, gPassAlbedo.a);
}