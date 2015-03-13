#version 410
layout(location=0) in vec4 position;
layout(location=1) in vec2 tex_coord;

out vec4 reflectedScreenPos;

uniform mat4 projView;
uniform mat4 projViewReflected;

void main() 
{
	reflectedScreenPos = projViewReflected * position;
	gl_Position = projView * position;
}