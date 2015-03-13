#version 410

in vec4 reflectedScreenPos;
out vec4 frag_color;
uniform sampler2D diffuse;

void main(){
	vec4 uvPosition = reflectedScreenPos / reflectedScreenPos.w;
	uvPosition = (uvPosition + 1) * 0.5f;
	frag_color = texture(diffuse, uvPosition.xy);
}