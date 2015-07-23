#version 410

in vec4 fPosition;
in vec4 fNormal;

layout(location = 0) out vec4 gPassAlbedo;
layout(location = 1) out vec4 gPassPosition;
layout(location = 2) out vec4 gPassNormal;
layout(location = 3) out vec4 gPassSpecular;

out vec4 fragColor;

void main() {
	gPassAlbedo = vec4(0.5);
	gPassPosition = vec4(fPosition.xyz, pow(gl_FragCoord.z, 25));
	gPassNormal = vec4(fNormal.xyz, 1.0f);
	gPassSpecular = vec4(vec3(0),2.0f);
}