#version 410
layout(location=0) in vec3 Position;
layout(location=1) in vec4 Normal;

out vec4 fragNormal;
out vec4 shadowCoord;

uniform mat4 projView;
uniform mat4 lightMatrix;

void main() 
{
	fragNormal = Normal;
	gl_Position = projView * vec4(Position, 1);
	shadowCoord = lightMatrix * vec4(Position, 1);
}