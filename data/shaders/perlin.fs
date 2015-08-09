#version 410

in vec2 fTexCoord;

out vec4 FragColor;

uniform sampler2D perlinTexture;
uniform float scale;

void main() {
	//FragColor = vec4(fTexCoord.x,0,fTexCoord.y,1);
	float perlinSample = texture(perlinTexture, fTexCoord).r ;

	if (perlinSample < 0.5) {
		FragColor = vec4(0, 0, 1, 1) * perlinSample / 2;
	}
	else if (perlinSample < 0.75) {
		FragColor = vec4(0.35, perlinSample, 0.1, 1) * perlinSample / 2;
	}
	else if (perlinSample < 0.95) {
		FragColor = vec4(perlinSample, 0.5, 0, 1) * perlinSample / 2;
	}
	else {
		FragColor = vec4(perlinSample, perlinSample, perlinSample, 1) * perlinSample / 2;
	}
} 