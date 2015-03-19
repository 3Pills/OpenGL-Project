#version 410
layout(location=0) in vec3 Position;
uniform mat4 lightMatrix;

void main() {
	gl_Position = lightMatrix * vec4(Position, 1);
}