#version 410

layout(location = 0) in vec4 Position;

uniform vec3 lightPos;
uniform float lightRadius;
uniform mat4 projView;

void main() {
	gl_Position = projView * vec4(Position.xyz * lightRadius + lightPos, 1);
}