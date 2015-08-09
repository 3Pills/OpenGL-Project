#version 410

in vec4 worldPos;
in vec4 worldNormal;

out vec4 fragColor;

void main() {
	vec3 color = vec3(1);
	if ( fract(worldPos.x) < 0.05
	  || fract(worldPos.y) < 0.05
	  || fract(worldPos.z) < 0.05 ) {
		color = vec3(0);
	}
	vec3 L = normalize(vec3(1));
	vec3 N = normalize(worldNormal.xyz);

	float d = max(0, dot(N,L)) * 0.75;
	fragColor.rgb = color * 0.25 + color * d;
	fragColor.a = 1;
}