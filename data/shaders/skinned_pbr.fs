#version 410
in vec4 fPosition;
in vec4 fNormal;
in vec3 fTangent;
in vec3 fBiTangent;
in vec2 fTexCoord;

out vec4 fragColor;

uniform vec3 ambCol;
uniform vec3 lightCol;
uniform vec3 lightDir;
uniform vec3 camPos;

uniform float roughness;
uniform float fresnelScale;

void main() {
	//Numerical Constants
	float e = 2.71828182845904523536028747135f;
	float pi = 3.1415926535897932384626433832f;

	//Correcting the model normals to the lit side of the model
	mat3 TBN = mat3(normalize(fTangent), normalize(fBiTangent), normalize(fNormal.xyz));
	vec3 sampledNormal = fNormal.xyz;
	vec3 adjustedNormal = sampledNormal * 2 - 1;

	//Oren-Nayer Start
	vec3 N = normalize(TBN * adjustedNormal);
	vec3 L = normalize(lightDir);
	vec3 E = normalize(camPos - fPosition.xyz);

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
	float CookTorrance = max((D*G*F) / (NdE * pi), 0.0f);

	vec4 A = vec4(vec3(lightCol + ambCol), 1);

	fragColor = A * (OrenNayer + CookTorrance);
}