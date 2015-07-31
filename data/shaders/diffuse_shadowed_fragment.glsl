#version 410

in vec4 fPosition;
in vec4 fNormal;
in vec4 shadowCoord;

out vec4 fragColor;

uniform vec3 lightDir;
uniform sampler2DShadow shadowMap;

float rand(vec4 vec, int number) {
	float dot_product = dot(vec * number, vec4(12.9898,78.233,45.164,94.673));
    return fract(sin(dot_product) * 43758.5453);
}

void main() { 
	//Random vectors
	vec2 poissonDisk[8] = vec2[](
		vec2( -0.74201624, -0.39906216 ),
		vec2( 0.64558609, -0.36890725 ),
		vec2( -0.044184101, -0.52938870 ),
		vec2( 0.14495938, 0.19387760 ),
		vec2( -0.4777346, -0.228894 ),
		vec2( 0.3227544, -0.276863 ),
		vec2( -0.235635, -0.15796734 ),
		vec2( 0.4153677, -0.515336 )
	);

	float d = max(0, dot(normalize(fNormal).xyz, lightDir));

	float bias = 0.005 * tan(acos(d));
	bias = clamp(bias, 0.f,0.01f);

	float distribution = 1.0f/9.0f;

	for (int i=0;i<8;i++) {
		int index = int(8.0*rand(fPosition, i))%8;
		if ( texture(shadowMap, vec3(shadowCoord.xy + poissonDisk[index]/400.0, shadowCoord.z - bias))  == 0.0f ){
			d -= distribution;
		}
	}
	if ( texture(shadowMap, vec3(shadowCoord.xy, shadowCoord.z - bias))  == 0.0f ){
		d -= distribution;
	}

	fragColor = vec4(d,d,d,1);
}