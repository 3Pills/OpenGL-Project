#version 410

layout(location = 0) in vec4 Position;
layout(location = 1) in vec2 TexCoord;

out vec2 vTexCoord;

uniform mat4 projView;
uniform sampler2D perlinTexture;
uniform float scale;

void main() {
	vec4 pos = Position;
	pos.y += texture(perlinTexture, TexCoord).r * scale;
	vTexCoord = TexCoord;
	gl_Position = projView * pos;
}