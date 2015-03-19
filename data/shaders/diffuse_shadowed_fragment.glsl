#version 410
in vec4 fragNormal;
in vec4 shadowCoord;

out vec4 fragColor;

uniform vec3 lightDir;
uniform sampler2D shadowMap;

void main() { 
	float d = max(0, dot(-lightDir, normalize(fragNormal).xyz));

	if (texture(shadowMap, vShadowCoord.xy).r < vShadowCoord.z) {
		d = 0;
	}

	fragColor = vec4(d,d,d,1);
}