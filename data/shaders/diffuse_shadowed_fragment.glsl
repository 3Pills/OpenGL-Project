#version 410

in vec4 fragNormal;
in vec4 shadowCoord;

out vec4 fragColor;

uniform vec3 lightDir;
uniform sampler2D shadowMap;

void main() { 
	float d = max(0, dot(normalize(fragNormal).xyz, lightDir));

	if (texture(shadowMap, shadowCoord.xy).r < shadowCoord.z - 0.1f) {
		d = 1;
	}

	fragColor = vec4(d,d,d,1);
}