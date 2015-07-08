#ifndef EMITTER_H_
#define EMITTER_H_

#include "glm_header.h"
#include "Vertex.h"
#include "Utility.h"

typedef enum EmitType {
	EMIT_POINT = 0,
	EMIT_LINE = 1,
	EMIT_PLANE = 2,
	EMIT_RECTANGLE = 3,
	EMIT_OUTER_RECTANGLE = 4,
	EMIT_RING = 5,
	EMIT_OUTER_RING = 6,
	EMIT_SPHERE = 7,
	EMIT_OUTER_SPHERE = 8,
	EMIT_COUNT = 9
};

struct Particle {
	vec4 position;
	vec4 color;
	float size;
	float lifetime;
	float lifespan;
	vec3 velocity;
};

class Emitter {
public:
	Emitter();
	~Emitter();

	void Init(unsigned int a_maxParticles, vec3 a_minPos, vec3 a_maxPos, EmitType a_emitType, float a_emitRate,
			  float a_lifespanMin, float a_lifespanMax, float a_velocityMin, float a_velocityMax,
			  float a_startSize, float a_endSize, vec4 a_startColor, vec4 a_endColor);


	void EmitParticles();
	void Update(float a_dt, mat4 a_camTransform);
	void Render();

	//Particle data
	Particle* m_particles;
	unsigned int m_maxParticles;
	unsigned int m_aliveCount;

	//OpenGL data
	OpenGLData m_buffers;
	VertexParticle* m_vertexData;
	unsigned int* m_indexData;

	//Emitter data
	vec4 m_minPos, m_maxPos;
	float m_emitRate, m_emitTimer;
	
	float m_lifespanMin, m_lifespanMax;
	float m_velocityMin, m_velocityMax;

	float m_startSize, m_endSize;
	vec4 m_startColor, m_endColor;

	EmitType m_emitType;
};

#endif//EMITTER_H_