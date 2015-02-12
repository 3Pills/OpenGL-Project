#version 410
in vec4 vColor;
in vec4 vPos;
out vec4 fragColor;
uniform float time;
void main() { 
	fragColor = vColor * abs(vPos.y) / 5;
	if (vPos.y > 0) {
		fragColor.x = abs(vPos.y / 20);
		fragColor.z = 0;
	}
	else {
		fragColor.x = 0;
		fragColor.z = abs(vPos.y / 20);
	}
	fragColor.y = 0.5 - abs(vPos.y)/20;
}