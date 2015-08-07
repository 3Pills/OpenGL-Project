#version 410

out vec4 LightOutput;

uniform vec3 lightViewPos;
uniform vec3 lightCol;

uniform float lightRadius;

uniform sampler2D positionTexture;
uniform sampler2D normalTexture;
uniform sampler2D specularTexture;
uniform samplerCube shadowMap;

uniform mat4 world;

float rand(vec4 vec, int number) {
	float dot_product = dot(vec * number, vec4(12.9898,78.233,45.164,94.673));
    return fract(sin(dot_product) * 43758.5453);
}

void main() {
	//Numerical Constants
	float e = 2.71828182845904523536028747135f;
	float pi = 3.1415926535897932384626433832f;
	
	//convert screenspace to texture coordinates
	vec2 texcoord = gl_FragCoord.xy / textureSize(positionTexture, 0).xy;

	//sample from textures
	vec4 positionSample = texture(positionTexture, texcoord);
	vec4 normalSample = texture(normalTexture, texcoord);
	vec4 specularSample = texture(specularTexture, texcoord);
	vec4 worldLightDir = (vec4(lightViewPos, 1) * world) - positionSample;
	
	float roughness = normalSample.a; //Roughness is stored in normal alpha
	float fresnelScale = specularSample.a; //Likewise, fresnel scale is stored in the specular alpha.
	
	//Set the w values to the correct value after sampling for other data
	positionSample.w = 1;
	normalSample.w = 0;

	//Oren-Nayer Start
	vec3 toLight = lightViewPos - positionSample.xyz;
	vec3 L = normalize(toLight);
	vec3 N = normalize(normalSample.xyz);
	vec3 E = -normalize(positionSample.xyz);

	float falloff = 1 - min(1, (length(toLight) / lightRadius));

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
	float X = 2.0f * NdH / dot( E, H );
	float G = min(1, min(X * NdE, X * NdL));

	//Final Cook-Torrance Equation
	float CookTorrance = max(0.0f, (D*G*F) / (NdE * pi));
	
	//Shadow-Mapping Start

	//Random offset vectors for shadow blurring
	vec2 offsetVectors[8] = vec2[](
		vec2( -0.75, -0.25 ),
		vec2( 0.65, -0.65 ),
		vec2( -0.05, -0.55 ),
		vec2( 0.15, 0.25 ),
		vec2( -0.50, -0.25 ),
		vec2( 0.45, 0.10 ),
		vec2( -0.25, 0.15 ),
		vec2( 0.40, -0.5 )
	);

	float d = OrenNayer;

	//Bias calculated
	float bias = 0.0005 * tan(acos(d));
	bias = clamp(bias, 0.f,0.01f);
	
	float distribution = 1.0f / 16.0f;
	float spreadFactor = 0.0004f;

	for (int i = 0; i < 8; i++) {
		int index = int(8.0 * rand(positionSample, i)) % 8;

		vec3 samplePoint = vec3(worldLightDir.xy + offsetVectors[index] * spreadFactor, worldLightDir.z);

		if ( texture(shadowMap, samplePoint).r > length(samplePoint) ){
			d -= distribution;
		}
	}
	if ( texture(shadowMap, worldLightDir.xyz).r > length(worldLightDir.xyz) ) {
		d -= distribution;
	}

	vec4 finalDiffuse = vec4(d,d,d,1);
	vec4 LightColor = vec4(lightCol, 1);

	LightOutput = LightColor * (finalDiffuse + CookTorrance) * falloff;
}