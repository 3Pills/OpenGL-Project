#version 410

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

out vec4 worldPos;
out vec4 worldNormal;

uniform mat4 projView;

void main() {
	worldPos = vec4(position, 1);
	worldNormal = vec4(normal, 0);

	gl_Position = projView * position;
}