#version 410

in vec2 fTexCoord;
in vec4 fColor;

out vec4 fragColor;

uniform sampler2D diffuse;

void main() {
	fragColor = fColor * texture(diffuse, fTexCoord);
}