#version 410

in vec2 fTexCoord;
out vec4 fColor;
uniform sampler2D diffuse;
uniform sampler2D normal;

void main(){
	fColor = texture(diffuse, fTexCoord);
	vec4 nColor = texture(normal, fTexCoord);
	fColor = fColor * nColor;
}