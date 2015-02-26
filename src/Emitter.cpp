#include "Emitter.h"
#include "gl_core_4_4.h"

Emitter::Emitter() :
	m_particles(nullptr), m_maxParticles(0), m_aliveCount(0),
	m_vertexData(nullptr), m_indexData(nullptr), m_minPos(0), m_maxPos(0),
	m_emitRate(0), m_emitTimer(0), m_lifespanMin(0), m_lifespanMax(0),
	m_velocityMin(0), m_velocityMax(0), m_startSize(0), m_endSize(0),
	m_startColor(0), m_endColor(0) {}

Emitter::~Emitter(){
	delete[] m_particles;
	delete[] m_vertexData;
	delete[] m_indexData;

	glDeleteVertexArrays(1, &m_buffers.m_VAO);
	glDeleteBuffers(1, &m_buffers.m_VBO);
	glDeleteBuffers(1, &m_buffers.m_IBO);
}

void Emitter::Init(unsigned int a_maxParticles, vec3 a_minPos, vec3 a_maxPos, EmitType a_emitType, 
float a_emitRate, float a_lifespanMin, float a_lifespanMax, float a_velocityMin, float a_velocityMax,
float a_startSize, float a_endSize, vec4 a_startColor, vec4 a_endColor) {
	m_maxParticles = a_maxParticles;
	m_minPos = vec4(a_minPos, 1);
	m_maxPos = vec4(a_maxPos, 1);
	m_emitRate = 1.0f / a_emitRate;
	m_lifespanMin = a_lifespanMin;
	m_lifespanMax = a_lifespanMax;
	m_velocityMin = a_velocityMin;
	m_velocityMax = a_velocityMax;
	m_startSize = a_startSize;
	m_endSize = a_endSize;
	m_startColor = a_startColor;
	m_endColor = a_endColor;
	m_emitType = a_emitType;

	m_particles = new Particle[m_maxParticles];
	m_vertexData = new VertexParticle[m_maxParticles * 4];
	m_indexData = new unsigned int[m_maxParticles * 6];

	for (unsigned int i = 0; i < m_maxParticles; ++i){
		unsigned int start = 4 * i;
		m_indexData[i * 6 + 0] = start + 0;
		m_indexData[i * 6 + 1] = start + 1;
		m_indexData[i * 6 + 2] = start + 2;

		m_indexData[i * 6 + 3] = start + 0;
		m_indexData[i * 6 + 4] = start + 2;
		m_indexData[i * 6 + 5] = start + 3;
	}

	glGenVertexArrays (1, &m_buffers.m_VAO);
	glGenBuffers(1, &m_buffers.m_VBO);
	glGenBuffers(1, &m_buffers.m_IBO);

	glBindVertexArray(m_buffers.m_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, m_buffers.m_VBO);
	glBufferData(GL_ARRAY_BUFFER, m_maxParticles * 4 * sizeof(VertexParticle), m_vertexData, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffers.m_IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_maxParticles * 6 * sizeof(unsigned int), m_indexData, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0); //Position
	glEnableVertexAttribArray(1); //Color

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(VertexParticle), 0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(VertexParticle), (void*)sizeof(vec4));

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Emitter::EmitParticles() {
	unsigned int particlesToEmit = (unsigned int)(m_emitTimer / m_emitRate);
	m_emitTimer -= particlesToEmit * m_emitRate;
	for (unsigned int i = 0; i < particlesToEmit && m_aliveCount < m_maxParticles; ++i) {
		vec4 planeMin = m_minPos;
		vec4 planeMax = m_maxPos;
		vec4 normal = vec4(0);
		switch (m_emitType){
		case (EMIT_POINT):
			m_particles[m_aliveCount].position = m_minPos + (m_maxPos - m_minPos) / 2;
			break;
		case (EMIT_LINE):
			m_particles[m_aliveCount].position = glm::mix(m_minPos, m_maxPos, (float)(rand() % 100) / 100);
			break;
		case (EMIT_PLANE):
			planeMin.y = m_minPos.y + (m_maxPos.y - m_minPos.y) / 2;
			planeMax.y = m_minPos.y + (m_maxPos.y - m_minPos.y) / 2;
			m_particles[m_aliveCount].position = glm::linearRand(planeMin, planeMax);
			break;
		case (EMIT_RECTANGLE):
			m_particles[m_aliveCount].position = glm::linearRand(m_minPos, m_maxPos);
			break;
		case (EMIT_OUTER_RECTANGLE) :
			normal = glm::normalize(glm::linearRand(m_minPos, m_maxPos));
			m_particles[m_aliveCount].position = normal * (m_maxPos - m_minPos).length() * 2;
			break;
		case (EMIT_RING) :
			m_particles[m_aliveCount].position = vec4(glm::diskRand((float)(m_maxPos - m_minPos).length() * 2), 0, 1);
			break;
		case (EMIT_OUTER_RING) :
			normal = glm::normalize(vec4(glm::diskRand((float)(m_maxPos - m_minPos).length() * 2), 0, 1));
			m_particles[m_aliveCount].position = normal * (m_maxPos - m_minPos).length() * 2;
			break;
		case (EMIT_SPHERE):
			m_particles[m_aliveCount].position = vec4(glm::ballRand((float)(m_maxPos-m_minPos).length()*2), 1);
			break;
		case (EMIT_OUTER_SPHERE) :
			normal = glm::normalize(vec4(glm::ballRand((float)(m_maxPos - m_minPos).length() * 2), 1));
			m_particles[m_aliveCount].position = normal * (m_maxPos - m_minPos).length() * 2;
			break;
		default:
			m_particles[m_aliveCount].position = m_minPos + (m_maxPos - m_minPos) / 2;
			break;
		}
		m_particles[m_aliveCount].lifetime = 0;
		m_particles[m_aliveCount].lifespan = glm::linearRand(m_lifespanMin, m_lifespanMax);

		m_particles[m_aliveCount].color = m_startColor;
		m_particles[m_aliveCount].size = m_startSize;
		float magnitude = glm::linearRand(m_velocityMin, m_velocityMax);
		m_particles[m_aliveCount].velocity = glm::sphericalRand(magnitude);

		++m_aliveCount;
	}
}

void Emitter::Update(float a_dt, mat4 a_camTransform) {
	m_emitTimer += a_dt;
	EmitParticles();

	for (unsigned int i = 0; i < m_aliveCount; ++i) {
		m_particles[i].lifetime += a_dt;
		if (m_particles[i].lifetime > m_particles[i].lifespan) {
			--m_aliveCount;
			m_particles[i] = m_particles[m_aliveCount];
		}

		m_particles[i].position.xyz += a_dt * m_particles[i].velocity;
		m_particles[i].velocity.y -= 0.5f;

		float t = m_particles[i].lifetime / m_particles[i].lifespan;
		m_particles[i].color = glm::mix(m_startColor, m_endColor, t);
		m_particles[i].size = glm::mix(m_startSize, m_endSize, t);

		mat4 particleTransform;

		vec3 f = glm::normalize(vec3(a_camTransform[3]) - vec3(m_particles[i].position));
		vec3 r = glm::cross(vec3(a_camTransform[1]), f);
		vec3 u = glm::cross(f, r);

		particleTransform[0].xyz = r * m_particles[i].size;
		particleTransform[1].xyz = u * m_particles[i].size;
		particleTransform[2].xyz = f * m_particles[i].size;
		particleTransform[3] = m_particles[i].position;

		m_vertexData[i * 4 + 0].position = particleTransform * vec4(-1, 1, 0, 1);
		m_vertexData[i * 4 + 1].position = particleTransform * vec4(-1,-1, 0, 1);
		m_vertexData[i * 4 + 2].position = particleTransform * vec4( 1,-1, 0, 1);
		m_vertexData[i * 4 + 3].position = particleTransform * vec4( 1, 1, 0, 1);

		m_vertexData[i * 4 + 0].color = m_particles[i].color;
		m_vertexData[i * 4 + 1].color = m_particles[i].color;
		m_vertexData[i * 4 + 2].color = m_particles[i].color;
		m_vertexData[i * 4 + 3].color = m_particles[i].color;
	}
}

void Emitter::Render() {
	glBindBuffer(GL_ARRAY_BUFFER, m_buffers.m_VBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, m_aliveCount * 4 * sizeof(VertexParticle), m_vertexData);

	glBindVertexArray(m_buffers.m_VAO);
	glDrawElements(GL_TRIANGLES, 6 * m_aliveCount, GL_UNSIGNED_INT, 0);
}