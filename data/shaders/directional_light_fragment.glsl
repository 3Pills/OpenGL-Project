#version 410

out vec4 LightOutput;

uniform vec3 lightDir;
uniform vec3 lightCol;
uniform vec3 ambCol;
uniform vec3 camPos;

uniform float specPow;

uniform sampler2D positionTexture;
uniform sampler2D normalTexture;
uniform sampler2D specularTexture;
uniform sampler2DShadow shadowMap;

uniform mat4 world;
uniform mat4 projView;
uniform mat4 lightMatrix;

float rand(vec4 vec, int number) {
	float dot_product = dot(vec * number, vec4(12.9898,78.233,45.164,94.673));
    return fract(sin(dot_product) * 43758.5453);
}

void main() {
	//Numerical Constants
	float e = 2.71828182845904523536028747135f;
	float pi = 3.1415926535897932384626433832f;
	
	//texture coord
	vec2 texcoord = gl_FragCoord.xy / textureSize(positionTexture, 0).xy;

	//sample from textures
	vec4 positionSample = texture(positionTexture, texcoord);
	vec4 normalSample = texture(normalTexture, texcoord);
	vec4 specularSample = texture(specularTexture, texcoord);
	vec4 shadowCoord = lightMatrix * world * positionSample;

	positionSample.w = 1;
	normalSample.w = 0;
	
	float roughness = normalSample.a; //Roughness is stored in normal alpha
	float fresnelScale = specularSample.a; //Likewise, fresnel scale is stored in the specular alpha.

	//Oren-Nayer Start
	vec3 L = normalize(-lightDir);
	vec3 N = normalize(normalSample.xyz);
	vec3 E = -normalize(positionSample.xyz);

	float NdL = max(0.0f, dot(N, L));
	float NdE = max(0.0f, dot(N, E));

	//Roughness calculation
	float R2 = roughness * roughness;
	float RA = 1.0f - 0.5f * R2 / (R2 + 0.33f);
	float RB = 0.45f * R2 / (R2 + 0.09f);

	// CX = max(0, cos(r,i))
	vec3 lightProjected = normalize( L - N * NdL );
	vec3 viewProjected = normalize( E - N * NdE);
	float CX = max( 0.0f, dot( lightProjected, viewProjected ) );

	// DX = sin(alpha) * tan(beta)
	float alpha = sin( max( acos( NdE ), acos( NdL ) ) );
	float beta = tan( min( acos( NdE ), acos( NdL ) ) );
	float DX = alpha * beta;

	//Final Oren-Nayer Equation
	float OrenNayer = NdL * (RA + RB * CX * DX);

	//Cook-Torrance Start
	vec3 H = normalize( L + E ); // light and view half vector
	float NdH = max(0.0f, dot(N, H));
	float NdH2 = NdH * NdH;

	//Beckmann Distribution
	float exponent = -(1 - NdH2) / (NdH2 * R2);
	float D = pow(e, exponent) / (R2 * NdH2 * NdH2);

	//Fresnel Term
	float HdE = dot( H, E );
	float F = mix( pow( 1 - HdE, 5 ), 1, fresnelScale);

	//Geometric Attenutation
	float X = (2.0f * NdH) / dot( E, H );
	float G = min(1, min(X * NdE, X * NdL));

	//Final Cook-Torrance Equation
	float CookTorrance = max(0.0f, (F*G*D) / (NdE * pi));

	//Shadow-Mapping Start

	//Random offset vectors for shadow blurring
	vec2 offsetVectors[8] = vec2[](
		vec2( -0.74201624, -0.39906216 ),
		vec2( 0.64558609, -0.36890725 ),
		vec2( -0.044184101, -0.52938870 ),
		vec2( 0.14495938, 0.19387760 ),
		vec2( -0.4777346, -0.228894 ),
		vec2( 0.3227544, -0.276863 ),
		vec2( -0.235635, -0.15796734 ),
		vec2( 0.4153677, -0.515336 )
	);

	float d = OrenNayer;

	//Bias calculated
	float bias = 0.005 * tan(acos(d));
	bias = clamp(bias, 0.f,0.01f);
	
	float distribution = 1.0f/18.0f;
	for (int i=0;i<8;i++) {
		int index = int(8.0*rand(positionSample, i))%8;
		if ( texture(shadowMap, vec3(shadowCoord.xy + offsetVectors[index]/1000.0, shadowCoord.z - bias))  == 0.0f ){
			d -= distribution;
		}
	}
	if ( texture(shadowMap, vec3(shadowCoord.xy, shadowCoord.z - bias)) == 0.0f ){
		d -= distribution;
	}
	//if (readShadowMap(lightProjected)) {
	//	d = 0;
	//}
	vec4 finalDiffuse = vec4(d,d,d,1);
	vec4 LightColor = vec4(lightCol, 1);
	//Use either ambient light hue or the OrenNayer calculations
	LightOutput = LightColor * (finalDiffuse + CookTorrance);
}