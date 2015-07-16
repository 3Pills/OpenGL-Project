#version 410

layout(location = 0) in vec4 Position;
layout(location = 1) in vec2 TexCoord;

out vec3 fPosition;
out vec3 fNormal;
out vec2 fTexCoord;

uniform mat4 projView;
uniform mat4 world;
uniform mat4 view;

uniform sampler2D perlinTexture;
uniform float scale;
uniform bool deferred;

void main() {
	vec3 pos = Position.xyz;
	pos.y += texture(perlinTexture, TexCoord).r * scale;

	fPosition = pos;
	fNormal = vec3(0,1,0);
	if (deferred) {
		fPosition = (view * vec4(fPosition, 1)).xyz;
		fNormal = (view * vec4(0,1,0, 0)).xyz;
	}

	fTexCoord = TexCoord;
	gl_Position = projView * vec4(pos,1);
}