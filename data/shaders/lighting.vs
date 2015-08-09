#version 410

layout(location=0) in vec4 Position;
layout(location=1) in vec4 Normal;

out vec4 vNormal;
out vec4 vPos;

uniform mat4 projView;

void main() { 
	vNormal = Normal;
	vPos = Position;
	gl_Position = projView * Position; 
}
