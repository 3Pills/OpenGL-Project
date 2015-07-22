#version 410

in vec4 fPosition;
in vec4 fNormal;
in vec2 fTexCoord;
in vec4 fColor;

layout(location = 0) out vec4 gPassAlbedo;
layout(location = 1) out vec4 gPassPosition;
layout(location = 2) out vec4 gPassNormal;
layout(location = 3) out vec4 gPassSpecular;

uniform sampler2D diffuse;
uniform sampler2D normal;
uniform sampler2D specular;

uniform float roughness;
uniform float fresnelScale;

void main() {
	gPassAlbedo = texture(diffuse, fTexCoord) * fColor;

	gPassPosition = vec4(fPosition.xyz, pow(gl_FragCoord.z, 25));

	vec4 finalNormal = texture(normal, fTexCoord) * fNormal;
	finalNormal.a = roughness;
	gPassNormal = finalNormal;
	
	vec4 finalSpecular = texture(specular, fTexCoord);
	finalSpecular.a = fresnelScale;
	gPassSpecular = finalSpecular;
}