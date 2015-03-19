#version 410
layout(location=0) in vec3 Position;
layout(location=1) in vec4 Normal;

out vec4 fragNormal;
out vec4 shadowCoord;

uniform mat4 ProjView;
uniform mat4 lightMatrix;

void main() 
{
	fragNormal = Normal;
	shadowCoord = lightMatrix * vec4(Position, 1);
	gl_Position = ProjView * vec4(Position, 1);
}