#ifndef GPUEMITTER_H_
#define GPUEMITTER_H_

#include "glm_header.h"
#include "Vertex.h"
#include "Utility.h"
#include "Camera.h"

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

typedef enum MoveType {
	PMOVE_LINEAR = 0,
	PMOVE_WAVE = 1,
	PMOVE_COUNT = 2
};

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

	void Init(vec3 a_pos = vec3(0), vec3 a_extents = vec3(1), unsigned int a_maxParticles = 100,
		float a_lifespanMin = 1.0f, float a_lifespanMax = 2.0f, float a_velocityMin = 1.0f, float a_velocityMax = 2.0f, float a_fadeIn = 0.0f, float a_fadeOut = 0.0f,
		float a_startSize = 1.0f, float a_endSize = 0.5f, vec4 a_startColor = vec4(1), vec4 a_endColor = vec4(1), 
		EmitType a_emitType = EmitType(0), MoveType a_moveType = MoveType(0), char* a_szFilename = "./data/textures/white.png");
	
	void Render(float a_dt, FlyCamera a_camera, bool a_deferred = false);
	void DrawDebugGizmos();

	void Reload();

	void CreateBuffers();
	void CreateTexture();

	//Particle data
	GPUParticle* m_particles;
	unsigned int m_maxParticles;

	//Emitter data
	vec3 m_pos;
	float m_lifespanMin, m_lifespanMax;
	float m_velocityMin, m_velocityMax;
	float m_startSize, m_endSize;
	float m_fadeIn, m_fadeOut;
	vec4 m_startColor, m_endColor;
	vec3 m_extents;

	unsigned int m_activeBuffer;
	unsigned int m_VAO[2], m_VBO[2];
	unsigned int m_instantRender, m_deferredRender, m_updateShader;
	unsigned int m_texture;

	char* m_szFilename;

	float m_lastDrawTime;

	EmitType m_emitType;
	MoveType m_moveType;
};

#endif//GPUEMITTER_H_