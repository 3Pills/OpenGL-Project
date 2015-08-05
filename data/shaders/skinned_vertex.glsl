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

uniform mat4 view;
uniform mat4 projView;
uniform mat4 transform;

const int MAX_BONES = 128;
uniform mat4 bones[MAX_BONES];

void main() 
{
	fPosition = transform * Position;
	fTexCoord = TexCoord;
	fNormal = transform * Normal;
	fTangent = (transform * Tangent).xyz;
	fBiTangent = cross(fNormal.xyz, fTangent);
	fColor = vec4(1);

	gl_Position = projView * transform * Position;
}