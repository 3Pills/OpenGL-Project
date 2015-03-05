#ifndef GPUEMITTER_H_
#define GPUEMITTER_H_

#include "glm_header.h"
#include "Vertex.h"
#include "Utility.h"
struct GPUParticle {
	GPUParticle() : lifetime(1), lifespan(0) {}
	glm::vec3 position;
	glm::vec3 velocity;
	float lifetime;
	float lifespan;
};

class GPUEmitter {
public:
	GPUEmitter();
	~GPUEmitter();

	void Init(unsigned int a_maxParticles, vec3 a_pos, EmitType a_emitType, float a_emitRate,
			  float a_lifespanMin, float a_lifespanMax, float a_velocityMin, float a_velocityMax,
			  float a_startSize, float a_endSize, vec4 a_startColor, vec4 a_endColor);

	void Render(float a_dt, mat4 a_camTransform, mat4 a_projView);

	void CreateBuffers();
	void CreateUpdateShader();
	void CreateDrawShader();

	//Particle data
	GPUParticle* m_particles;
	unsigned int m_maxParticles;

	//Emitter data
	vec4 m_pos;
	float m_emitRate, m_emitTimer;
	
	float m_lifespanMin, m_lifespanMax;
	float m_velocityMin, m_velocityMax;
	float m_startSize, m_endSize;
	vec4 m_startColor, m_endColor;

	unsigned int m_activeBuffer;
	unsigned int m_VAO[2], m_VBO[2];
	unsigned int m_drawShader, m_updateShader;

	float m_lastDrawTime;

	EmitType m_emitType;
};

#endif//GPUEMITTER_H_