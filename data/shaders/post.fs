#version 410
in vec2 fTexCoord;
out vec4 FragColour;

uniform sampler2D target;
uniform float time;
uniform int screenWidth;
uniform int screenHeight;

vec4 Simple() {
	return texture(target, fTexCoord);
}

vec4 BoxBlur() {
	vec2 texel = 1.0f / textureSize(target, 0).xy;
	// 9-tap box kernel
	vec4 colour = texture(target, fTexCoord);
	colour += texture(target, fTexCoord + vec2(-texel.x, texel.y));
	colour += texture(target, fTexCoord + vec2(-texel.x, 0));
	colour += texture(target, fTexCoord + vec2(-texel.x, -texel.y));
	colour += texture(target, fTexCoord + vec2(0, texel.y));
	colour += texture(target, fTexCoord + vec2(0, -texel.y));
	colour += texture(target, fTexCoord + vec2(texel.x, texel.y));
	colour += texture(target, fTexCoord + vec2(texel.x, 0));
	colour += texture(target, fTexCoord + vec2(texel.x, -texel.y));

	return colour / 9;
}

vec4 GaussianBlur() {
	vec2 texel = 1.0f / textureSize(target, 0).xy;

	float quarter = 1.0f / 4.0f;
	float eighth = quarter / 2.0f;
	float sixteenth = eighth / 2.0f;

	vec4 colour = texture(target, fTexCoord) * quarter;

	colour += texture(target, fTexCoord + vec2(-texel.x, 0)) * eighth;
	colour += texture(target, fTexCoord + vec2(0, texel.y))	 * eighth;
	colour += texture(target, fTexCoord + vec2(0, -texel.y)) * eighth;
	colour += texture(target, fTexCoord + vec2(texel.x, 0))	 * eighth;

	colour += texture(target, fTexCoord + vec2(-texel.x, texel.y))	* sixteenth;
	colour += texture(target, fTexCoord + vec2(-texel.x, -texel.y))	* sixteenth;
	colour += texture(target, fTexCoord + vec2(texel.x, texel.y))	* sixteenth;
	colour += texture(target, fTexCoord + vec2(texel.x, -texel.y))	* sixteenth;

	return colour;
}

vec4 Distort(int num) {
	vec2 mid = vec2(0.5f);
	float distanceFromCentre = distance(fTexCoord, mid);
	vec2 normalizedCoord = normalize(fTexCoord - mid);
	float bias = distanceFromCentre + ((num == 0) ? sin(distanceFromCentre * 15) * 0.02f : cos(distanceFromCentre * 15) * 0.02f);
	vec2 newCoord = mid + bias * normalizedCoord;
	newCoord -= mid;
	newCoord *= 0.9f;
	newCoord += mid;
	return texture(target, newCoord);
}

vec4 Distort2(int num) {
	vec2 mid = vec2(0.5f);
	float distanceFromCentre = distance(fTexCoord, mid);
	vec2 normalizedCoord = normalize(fTexCoord - mid);
	float bias = distanceFromCentre + ((num == 0) ? sin(distanceFromCentre * 15) * 0.02f : cos(distanceFromCentre * 15) * 0.02f);
	vec2 newCoord = mid + bias * normalizedCoord;
	newCoord -= mid;
	newCoord *= 0.9f;
	newCoord += mid;
	return texture(target, newCoord);
}

vec4 Symmetry() {
	vec2 mid = vec2(0.5f);
	vec2 midCoord = fTexCoord - mid;
	vec2 newCoord = vec2(abs(midCoord.x), abs(midCoord.y));
	newCoord += mid;
	return texture(target, newCoord);
}

vec4 SinWave() {
	vec2 mid = vec2(0.5f);
	vec2 midCoord = fTexCoord - mid;
	vec2 newCoord = vec2(midCoord.x * midCoord.y, midCoord.y * midCoord.x);
	newCoord += mid;
	return texture(target, newCoord);
}	

vec4 ScanLines() {
	vec4 color = texture(target, fTexCoord);
	vec4 newColor = vec4(fract(fTexCoord.y*(screenHeight/2)));
	vec4 newColor2 = vec4(fract(fTexCoord.x*(screenWidth/2)));
	if (newColor.y < 0.5)
		color = vec4(0);
	//if (newColor2.x < 0.5)
	//	color = vec4(0);
	return (color);
}

void main() {
	FragColour = ScanLines();
}