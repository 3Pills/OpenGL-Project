#version 410

layout(location = 0) in vec4 Position;
layout(location = 1) in vec2 TexCoord;

out vec4 fPosition;
out vec4 fNormal;
out vec2 fTexCoord;

uniform mat4 projView;
uniform mat4 world;
uniform mat4 view;

uniform sampler2D perlinTexture;
uniform float scale;
uniform bool deferred;

float getHeight(vec2 texCoord) {
	return texture(perlinTexture, vec2(texCoord.x, texCoord.y)).r * scale;
}

vec4 getNormal() {
	float delta = 0.01;

	vec3 tangentx = vec3(TexCoord.x + delta, TexCoord.y, 0);
	vec3 tangentz = vec3(TexCoord.x, TexCoord.y + delta, 0);

	float samplex = getHeight(tangentx.xy);
	float samplez = getHeight(tangentz.xy);

	tangentx.z += samplex;
	tangentz.z += samplez;

	return vec4(normalize(cross(tangentx, tangentz)), 1);
}

void main() {
	vec4 pos = Position;
	pos.y += getHeight(TexCoord);

	fPosition = pos;
	fNormal = vec4(0,1,0,0);
	if (deferred) {
		fPosition = (view * fPosition);
		fNormal = (view * fNormal);
	}

	fTexCoord = TexCoord;
	gl_Position = projView * pos;
}