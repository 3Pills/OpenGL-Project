#version 410

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoord;
in vec4 fColor;

out vec4 fragColor;

uniform sampler2D diffuse;

void main() {
	fragColor = fColor * texture(diffuse, fTexCoord);
}