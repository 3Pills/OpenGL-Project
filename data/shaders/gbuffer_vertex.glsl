#version 410

layout(location = 0) in vec4 Position;
layout(location = 1) in vec4 Normal;

out vec4 fPosition;
out vec4 fNormal;

uniform mat4 view;
uniform mat4 projView;

void main() {
	fPosition = (view * vec4(Position.xyz, 1));
	fNormal = (view * vec4(Normal.xyz, 0));

	gl_Position = projView * vec4(Position.xyz, 1);
} 