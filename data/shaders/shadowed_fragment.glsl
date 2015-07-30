#version 410

out float fragDepth;

void main(){
	fragDepth = pow(gl_FragCoord.z, 10);
}