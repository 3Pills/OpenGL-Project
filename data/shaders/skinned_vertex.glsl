#version 410
layout(location=0) in vec4 Position;
layout(location=1) in vec2 TexCoord;
layout(location=2) in vec4 Indices;
layout(location=3) in vec4 Weights;
layout(location=4) in vec4 Normal;
layout(location=5) in vec4 Tangent;

out vec3 fPos;
out vec2 fTexCoord;
out vec3 fNormal;
out vec3 fTangent;
out vec3 fBiTangent;

uniform mat4 projView;
uniform mat4 world;

const int MAX_BONES = 128;
uniform mat4 bones[MAX_BONES];

void main() 
{
	fPos = Position.xyz;
	fNormal = Normal.xyz;
	fTangent = Tangent.xyz;
	fBiTangent = cross(Normal.xyz, Tangent.xyz);
	fTexCoord = TexCoord;

	ivec4 indices = ivec4(Indices);
	vec4 finalPos = bones[indices.x] * Position * Weights.x;
	finalPos += bones[indices.y] * Position * Weights.y;
	finalPos += bones[indices.z] * Position * Weights.z;
	finalPos += bones[indices.w] * Position * Weights.w;

	finalPos.w = 1;

	gl_Position = projView * world * finalPos;
}