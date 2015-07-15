#version 410

layout(location=0) in vec3 Position;
layout(location=1) in vec3 Velocity;
layout(location=2) in float Lifetime;
layout(location=3) in float Lifespan;

out vec3 vPosition;
out float vLifetime;
out float vLifespan;

void main() {
	vPosition = Position;
	vLifetime = Lifetime;
	vLifespan = Lifespan;
}