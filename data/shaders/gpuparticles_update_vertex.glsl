#version 410

layout(location=0) in vec3 Position;
layout(location=1) in vec3 Velocity;
layout(location=2) in float Lifetime;
layout(location=3) in float Lifespan;

out vec3 position;
out vec3 velocity;
out float lifetime;
out float lifespan;

uniform float deltaTime;
uniform vec3 emitterPos;

uniform float min_velocity;
uniform float max_velocity;
uniform float min_lifespan;
uniform float max_lifespan;

uniform float time;

const float INVERSE_MAX_UINT = 1.0f / 4294967295.0f;

float rand(uint seed, float range) {
	uint i = (seed ^ 12345391u) * 2654435769u;
	i ^= (i << 6u) ^ (i >> 26u);
	i *= 2654435769u;
	i += (i << 5u) ^ (i >> 12u);
	return float(range * i) * INVERSE_MAX_UINT;
} 

void main() {
	position = Position + Velocity * deltaTime;
	velocity = Velocity;
	lifetime = Lifetime + deltaTime;
	lifespan = Lifespan;

	// emit a new particle as soon as it dies
	if (lifetime > lifespan) {
		position = emitterPos;
		uint seed = uint(time * 1000.0) + uint(gl_VertexID);
		
		//set its velocity
		float vel_len = rand(seed++, max_velocity - min_velocity) + min_velocity;

		velocity.x = rand(seed++, 2) - 1;
		velocity.y = rand(seed++, 2) - 1;
		velocity.z = rand(seed++, 2) - 1;
		velocity = normalize(velocity) * vel_len;

		lifetime = 0;
		lifespan = rand(seed++, max_lifespan - min_lifespan) + min_lifespan;
	}
}