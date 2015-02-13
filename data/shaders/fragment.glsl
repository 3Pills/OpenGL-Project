#version 410
in vec4 vColor;
in vec4 vPos;
out vec4 fragColor;
uniform float time;
void main() { 
	fragColor = vColor;
}