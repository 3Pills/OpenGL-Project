#version 410
layout(location=0) in vec4 Position;
layout(location=1) in vec2 TexCoord;
layout(location=2) in vec4 Indices;
layout(location=3) in vec4 Weights;
layout(location=4) in vec4 Normal;
layout(location=5) in vec4 Tangent;

out vec3 fPosition;
out vec3 fNormal;
out vec3 fTangent;
out vec3 fBiTangent;
out vec2 fTexCoord;
out vec4 fColor;

uniform bool deferred;

uniform mat4 projView;
uniform mat4 view;
uniform mat4 world;

const int MAX_BONES = 128;
uniform mat4 bones[MAX_BONES];

void main() 
{
	fPosition = Position.xyz;
	fTexCoord = TexCoord;
	fNormal = Normal.xyz;
	fTangent = Tangent.xyz;
	fColor = vec4(1);

	ivec4 indices = ivec4(Indices);
	vec4 finalPos;
	finalPos =  (bones[indices.x] * Position) * Weights.x;
	finalPos += (bones[indices.y] * Position) * Weights.y;
	finalPos += (bones[indices.z] * Position) * Weights.z;
	finalPos.w = 1;
	
	vec4 finalNormal;
	finalNormal =  (bones[indices.x] * Normal) * Weights.x;
	finalNormal += (bones[indices.y] * Normal) * Weights.y;
	finalNormal += (bones[indices.z] * Normal) * Weights.z;
	finalNormal.w = 0;
	fNormal = finalNormal.xyz;
	
	vec4 finalTangent;
	finalTangent =  (bones[indices.x] * Tangent) * Weights.x;
	finalTangent += (bones[indices.y] * Tangent) * Weights.y;
	finalTangent += (bones[indices.z] * Tangent) * Weights.z;
	finalTangent.w = 0;
	fTangent = finalTangent.xyz;
	
	fBiTangent = cross(fNormal, fTangent);
	
	if (deferred) {
		fPosition = (view * world * finalPos).xyz;
		fNormal = (view * finalNormal).xyz;
	}

	gl_Position = projView * world * finalPos;
}