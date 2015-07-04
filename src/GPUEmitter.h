#ifndef GPUEMITTER_H_
#define GPUEMITTER_H_

#include "glm_header.h"
#include "Vertex.h"
#include "Utility.h"
struct GPUParticle {
	GPUParticle() : lifetime(1) {}
	glm::vec3 position;
	glm::vec3 velocity;
	float lifetime;
	float lifespan;
};

class GPUEmitter {
public:
	GPUEmitter();
	~GPUEmitter();

	void Init(vec3 a_pos = vec3(0, 0, 0), unsigned int a_maxParticles = 100,
		float a_lifespanMin = 1.0f, float a_lifespanMax = 2.0f, float a_velocityMin = 1.0f, float a_velocityMax = 2.0f,
		float a_startSize = 1.0f, float a_endSize = 0.5f, vec4 a_startColor = vec4(1, 1, 1, 1), vec4 a_endColor = vec4(1, 1, 1, 1), 
		EmitType a_emitType = EmitType(0), char* a_szFilename = "./data/textures/white.png");
	
	void Render(float a_dt, mat4 a_camTransform, mat4 a_projView);

	void CreateBuffers();
	void CreateTexture();

	//Particle data
	GPUParticle* m_particles;
	unsigned int m_maxParticles;

	//Emitter data
	vec4 m_pos;
	float m_lifespanMin, m_lifespanMax;
	float m_velocityMin, m_velocityMax;
	float m_startSize, m_endSize;
	vec4 m_startColor, m_endColor;

	unsigned int m_activeBuffer;
	unsigned int m_VAO[2], m_VBO[2];
	unsigned int m_drawShader, m_updateShader;
	unsigned int m_texture;

	char* m_szFilename;

	float m_lastDrawTime;

	EmitType m_emitType;
};

#endif//GPUEMITTER_H_