#version 410
layout(location=0) in vec3 Position;
uniform mat4 lightMatrix;
uniform mat4 projView;

void main() {
	gl_Position = projView * lightMatrix * vec4(Position, 1);
}