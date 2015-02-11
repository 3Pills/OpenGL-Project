#version 410
layout(location = 0) in vec4 Position;
layout(location=1) in vec4 Color;
out vec4 vColor;
uniform mat4 ProjectionView;
uniform float time;
uniform float heightScale;
void main() {
	vColor = Color;
	vec4 P = Position;
	P.y += cos(time*2 + (P.x - 50)/10) * heightScale;
	P.y += cos(time*3 + (P.z - 50)/8) * heightScale;
	gl_Position = ProjectionView * P;
}