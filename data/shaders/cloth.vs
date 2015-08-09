#version 410

layout(location = 0) in vec4 Position;
layout(location = 1) in vec2 TexCoord;
layout(location = 2) in vec4 Normal;

out vec4 fPosition;
out vec4 fNormal;
out vec2 fTexCoord;
out vec4 fColor;

uniform mat4 projView;

void main() {
	fPosition = Position;
	fNormal = Normal;
	fTexCoord = TexCoord;
	fColor = vec4(1);

	gl_Position = projView * vec4(Position.xyz, 1);
} 