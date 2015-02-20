#version 410
layout(location=0) in vec4 Position;
layout(location=1) in vec2 TexCoord;
layout(location=2) in vec4 Indices;
layout(location=3) in vec4 Weights;

out vec2 fTexCoord;
uniform mat4 ProjectionView;
uniform mat4 world;

const int MAX_BONES = 128;
uniform mat4 bones[MAX_BONES];

void main() 
{
	fTexCoord = TexCoord;

	ivec4 indices = ivec4(Indices);
	vec4 FinalPosition = bones[indices.x] * Position * Weights.x;
	FinalPosition += bones[indices.y] * Position * Weights.y;
	FinalPosition += bones[indices.z] * Position * Weights.z;
	FinalPosition += bones[indices.w] * Position * Weights.w;

	FinalPosition.w = 1;

	gl_Position = ProjectionView * world * FinalPosition;
}