#version 410

layout(location=0) in vec4 Position;
layout(location=1) in vec4 Normal;
layout(location=2) in vec4 Tangent;
layout(location=3) in vec2 TexCoord;

out vec3 fPos;
out vec3 fNormal;
out vec3 fTangent;
out vec3 fBiTangent;
out vec2 fTexCoord;

uniform mat4 ProjectionView;

void main() { 
	fPos = Position.xyz;
	fNormal = Normal.xyz;
	fTangent = Tangent.xyz;
	fBiTangent = cross(Normal.xyz, Tangent.xyz);
	fTexCoord = TexCoord;

	gl_Position = ProjectionView * Position; 
}