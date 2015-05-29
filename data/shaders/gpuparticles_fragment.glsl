#version 410

in vec4 vColor;
out vec4 fragColor;
in vec2 fragTexCoord;

in vec3 position;
in float lifetime;
in float lifespan;

uniform sampler2D diffuse;

void main() {
	vec4 dix = texture(diffuse, fragTexCoord);
	dix.x = dix.x;
	dix.y = dix.y;
	dix.z = dix.z;
	fragColor = vColor * dix;
}