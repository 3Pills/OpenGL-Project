#version 410
layout(location=0) in vec4 Position;
layout(location=1) in vec2 TexCoord;
layout(location=2) in vec4 Indices;
layout(location=3) in vec4 Weights;
layout(location=4) in vec4 Normal;
layout(location=5) in vec4 Tangent;

out vec4 fPosition;
out vec4 fNormal;
out vec3 fTangent;
out vec3 fBiTangent;
out vec2 fTexCoord;
out vec4 fColor;

uniform mat4 projView;
uniform mat4 transform;

const int MAX_BONES = 128;
uniform mat4 bones[MAX_BONES];

void main() {
	fTexCoord = TexCoord;
	fColor = vec4(1);

	ivec4 indices = ivec4(Indices);
	vec4 finalPosition = (bones[indices.x] * Position) * Weights.x;
	finalPosition += (bones[indices.y] * Position) * Weights.y;
	finalPosition += (bones[indices.z] * Position) * Weights.z;
	finalPosition.w = 1;
	finalPosition = transform * finalPosition;
	
	vec4 finalNormal = (bones[indices.x] * Normal) * Weights.x;
	finalNormal += (bones[indices.y] * Normal) * Weights.y;
	finalNormal += (bones[indices.z] * Normal) * Weights.z;
	finalNormal.w = 0;
	finalNormal = transform * finalNormal;
	
	vec4 finalTangent = (bones[indices.x] * Tangent) * Weights.x;
	finalTangent += (bones[indices.y] * Tangent) * Weights.y;
	finalTangent += (bones[indices.z] * Tangent) * Weights.z;
	finalTangent.w = 0;
	finalTangent = transform * finalTangent;
	fBiTangent = cross(finalNormal.xyz, finalTangent.xyz);

	fPosition = finalPosition;
	fNormal = finalNormal;
	fTangent = (finalTangent).xyz;

	gl_Position = projView * finalPosition;
}