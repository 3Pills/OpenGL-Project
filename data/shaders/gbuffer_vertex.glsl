#version 410

layout(location = 0) in vec4 Position;
layout(location = 1) in vec4 Normal;

out vec3 fPosition;
out vec3 fNormal;

uniform mat4 view;
uniform mat4 projView;

void main() {
	fPosition = (view * vec4(Position.xyz, 1)).xyz;
	fNormal = (view * vec4(Normal.xyz, 0)).xyz;

	gl_Position = projView * vec4(Position.xyz, 1);
} 