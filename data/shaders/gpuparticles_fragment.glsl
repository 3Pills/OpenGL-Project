#version 410

in vec4 vColor;
out vec4 fragColor;
in vec2 fragTexCoord;

in vec3 position;
in float lifetime;
in float lifespan;

uniform sampler2D diffuse;

void main() {
	vec4 color = vColor * texture(diffuse, fragTexCoord);
	color.a = vColor.a * texture(diffuse, fragTexCoord).a;
	fragColor = color;
}