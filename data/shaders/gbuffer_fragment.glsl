#version 410

in vec4 fPosition;
in vec4 fNormal;
in vec2 fTexCoord;

layout(location = 0) out vec4 gPassAlbedo;
layout(location = 1) out vec4 gPassPosition;
layout(location = 2) out vec4 gPassNormal;
layout(location = 3) out vec4 gPassSpecular;
layout(location = 4) out vec4 gPassWorldPos;

uniform sampler2D perlinTexture;
uniform mat4 view;

out vec4 fragColor;

void main() {
	gPassAlbedo = vec4(1);
	gPassWorldPos = fPosition;
	gPassPosition = view * fPosition;
	gPassNormal = vec4((view * fNormal).xyz, 1.0f);
	gPassSpecular = vec4(vec3(0),2.0f);
}