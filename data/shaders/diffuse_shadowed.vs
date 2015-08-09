#version 410
layout(location=0) in vec3 Position;
layout(location=1) in vec4 Normal;
layout(location=2) in vec4 Tangent;
layout(location=3) in vec2 TexCoord;

out vec4 fPosition;
out vec4 fNormal;
out vec2 fTexCoord;

uniform mat4 projView;

void main() 
{
	fPosition = vec4(Position, 1);
	fNormal = Normal;
	fTexCoord = TexCoord;
	gl_Position = projView * vec4(Position, 1);
}