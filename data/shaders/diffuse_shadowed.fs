#version 410

in vec4 fPosition;
in vec4 fNormal;
in vec4 shadowCoord;
in vec2 fTexCoord;

out vec4 fragColor;

uniform vec3 lightDir;
uniform sampler2DShadow shadowMap;
uniform sampler2D shadowTexture;
uniform mat4 lightMatrix;

float rand(vec4 vec, int number) {
	float dot_product = dot(vec * number, vec4(12.9898,78.233,45.164,94.673));
    return fract(sin(dot_product) * 43758.5453);
}

void main() { 
	//Random vectors
	vec2 poissonDisk[8] = vec2[](
		vec2( -0.75, -0.25 ),
		vec2( 0.65, -0.65 ),
		vec2( -0.05, -0.55 ),
		vec2( 0.15, 0.25 ),
		vec2( -0.50, -0.25 ),
		vec2( 0.45, 0.10 ),
		vec2( -0.25, 0.15 ),
		vec2( 0.40, -0.5 )
	);

	vec4 shadowCoord = lightMatrix * fPosition;

	float d = max(0, dot(normalize(fNormal).xyz, -lightDir));

	float bias = 0.005 * tan(acos(d));
	bias = clamp(bias, 0.f,0.025f);

	float distribution = 1.0f/9.0f;

	for (int i=0;i<8;i++) {
		int index = int(8.0*rand(fPosition, i))%8;
		if ( texture(shadowMap, vec3(shadowCoord.xy + poissonDisk[index] / 700.0f, shadowCoord.z - bias), bias)  == 0.0f ){
			d -= distribution;
		}
	}
	if ( texture(shadowMap, vec3(shadowCoord.xy, shadowCoord.z - bias), bias)  == 0.0f ){
		d -= distribution;
	}
	fragColor = vec4(d,d,d,1);
	//fragColor = texture(shadowTexture, vec2(fTexCoord.xy));
}