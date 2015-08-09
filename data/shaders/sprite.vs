#version 410

layout(location = 0) in vec4 Position;
layout(location = 1) in vec2 TexCoord;
layout(location = 2) in vec4 Color;

out vec4 fPosition;
out vec2 fTexCoord;
out vec4 fColor;

uniform mat4 view;
uniform mat4 projView;

void main() {
	fPosition = (view * vec4(Position.xyz, 1));
	fTexCoord = TexCoord;
	fColor = Color;

	gl_Position = projView * vec4(Position.xyz, 1);
} 