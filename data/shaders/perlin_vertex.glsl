#version 410

layout(location = 0) in vec4 Position;
layout(location = 1) in vec2 TexCoord;

out vec4 fPosition;
out vec4 fNormal;
out vec2 fTexCoord;

uniform mat4 projView;
uniform mat4 transform;

uniform sampler2D perlinTexture;
uniform float scale;
uniform bool deferred;

uniform vec2 worldSize;
uniform vec2 textureSize;

float getHeight(vec2 texCoord) {
	return texture(perlinTexture, vec2(texCoord.x, texCoord.y)).r * scale;
}

vec4 getNormal(vec3 pos) {
	float deltaX = 1.0f / textureSize.x;
	float deltaY = 1.0f / textureSize.y;
	float worldDeltaX = worldSize.x / textureSize.x;
	float worldDeltaY = worldSize.y / textureSize.y;

	vec2 coordXP = vec2(TexCoord.x + deltaX, TexCoord.y);
	vec2 coordXN = vec2(TexCoord.x - deltaX, TexCoord.y);
	vec2 coordZP = vec2(TexCoord.x, TexCoord.y + deltaY);
	vec2 coordZN = vec2(TexCoord.x, TexCoord.y - deltaY);

	vec3 tangentXP = vec3(pos.x + worldDeltaX, pos.y + getHeight(coordXP), pos.z);
	vec3 tangentXN = vec3(pos.x - worldDeltaX, pos.y + getHeight(coordXN), pos.z);
	vec3 tangentZP = vec3(pos.x, pos.y + getHeight(coordZP), pos.z + worldDeltaY);
	vec3 tangentZN = vec3(pos.x, pos.y + getHeight(coordZN), pos.z - worldDeltaY);

	return vec4(normalize(cross(tangentZP - tangentZN, tangentXP - tangentXN)), 0);
}

void main() {
	vec4 pos = Position;
	fNormal = getNormal(pos.xyz);
	pos.y += getHeight(TexCoord);
	fPosition = pos;

	fPosition = (transform * fPosition);
	fNormal = transform * fNormal;

	fTexCoord = TexCoord;
	gl_Position = projView * transform * pos;
}