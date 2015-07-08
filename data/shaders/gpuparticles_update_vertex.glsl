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

uniform int moveType;
uniform int emitType;

uniform vec3 emitPos;

uniform float minVel;
uniform float maxVel;
uniform float minLife;
uniform float maxLife;

uniform float time;
uniform vec3 extents;

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
	
	//Wont do anything if movetype is 0.
	position += vec3(0, moveType * (cos(lifetime*7) / 200), 0);

	// emit a new particle as soon as it dies
	if (lifetime > lifespan) {
		uint seed = uint(time * 1000.0) + uint(gl_VertexID);

		position = emitPos;
		vec3 planeRand = vec3(0);

		if (emitType == 1) {
			planeRand = vec3(rand(seed, extents.x), rand(seed, extents.y), rand(seed, extents.z));
		}
		if (emitType > 1) {
			planeRand = vec3(rand(seed++, extents.x * 2) - extents.x, rand(seed++, extents.y * 2) - extents.y, rand(seed++, extents.z * 2) - extents.z);
		}
		if (emitType == 2 || emitType == 5 || emitType == 6) {
			planeRand.y = 0;
		}
		if (emitType == 4) {
			planeRand = vec3(rand(seed++, extents.x * 2) - extents.x, rand(seed++, extents.y * 2) - extents.y, 0);
			vec3 extentsR = extents;
			int randReverse = int(round(rand(seed++,2)));
			if (randReverse == 1) {
				planeRand = -planeRand;
				extentsR = -extents;
			}
			
			int randSide = int(round(rand(seed++, 6)));
			if (randSide == 1) {
				planeRand = vec3(extentsR.x, planeRand.xy);
			}
			else if (randSide == 2) {
				planeRand = vec3(planeRand.x, extentsR.y, planeRand.y);
			}
			else if (randSide == 3) {
				planeRand = vec3(planeRand.y, extentsR.y, planeRand.x);
			}
			else if (randSide == 4) {
				planeRand = vec3(extentsR.x, planeRand.y, planeRand.x);
			}
			else if (randSide == 5) {
				planeRand = vec3(planeRand.y, planeRand.x, extentsR.z);
			}
			else {
				planeRand = vec3(planeRand.xy, extentsR.z);
			}
		}
		if (emitType > 4) {
			planeRand = normalize(planeRand);
		}
		if (emitType == 5 || emitType == 7) {
			planeRand *= rand(seed++, length(extents.x));
		}
		if (emitType == 6 || emitType == 8) {
			planeRand *= length(extents.x);
		}
		position = planeRand;
		
		//set its velocity
		float velLen = rand(seed++, maxVel - minVel) + minVel;

		if (moveType == 1) {
			velocity = vec3(rand(seed++, 2) - 1, 0, rand(seed++, 2) - 1);
		}
		else {
			velocity = vec3(rand(seed++, 2) - 1, rand(seed++, 2) - 1, rand(seed++, 2) - 1);
		}
		velocity = normalize(velocity) * velLen;

		lifetime = 0;
		lifespan = rand(seed++, maxLife - minLife) + minLife;
	}
}