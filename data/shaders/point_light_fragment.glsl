#version 410

out vec3 LightOutput;

uniform vec3 lightViewPos;
uniform vec3 lightColor;
uniform float lightRadius;

uniform sampler2D positionTexture;
uniform sampler2D normalTexture;

void main() {
	//texture coord
	vec2 texcoord = gl_FragCoord.xy / textureSize(positionTexture, 0).xy;

	//sample from textures
	vec4 positionSample = texture(positionTexture, texcoord);
	vec4 normalSample = texture(normalTexture, texcoord);

	//compute light direction
	vec3 toLight = lightViewPos - positionSample.xyz;
	vec3 L = normalize(toLight);

	//compute lambertian term
	float d = max(0, dot(normalSample.xyz, L));

	//compute falloff
	float falloff = 1 - min(1, (length(toLight) / lightRadius));

	//output lambert * falloff * color
	LightOutput = (d * falloff * lightColor).xyz;
}