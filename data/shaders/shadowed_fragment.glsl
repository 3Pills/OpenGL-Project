#version 410

out float fragDepth;
out vec4 fragColor;

void main(){
	fragDepth = gl_FragCoord.z;
	fragColor = vec4(gl_FragCoord.z);
}