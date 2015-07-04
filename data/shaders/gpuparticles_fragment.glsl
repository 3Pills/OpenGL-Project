#version 410

in vec4 vColor;
out vec4 fragColor;
in vec2 fragTexCoord;

in vec3 position;
in float lifetime;
in float lifespan;

void main() {
	fragColor = vColor;
}