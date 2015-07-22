#include "GPUEmitter.h"
#include "gl_core_4_4.h"
#include "stb_image.h"
#include <string>
#include <Gizmos.h>

GPUEmitter::GPUEmitter() :
	m_particles(nullptr), m_maxParticles(0), m_pos(0), m_lifespanMin(0), m_lifespanMax(0),
	m_velocityMin(0), m_velocityMax(0), m_startSize(0), m_endSize(0),
	m_startColor(0), m_endColor(0), m_instantRender(0), m_deferredRender(0), m_updateShader(0),
	m_lastDrawTime(0), m_activeBuffer(0), m_szFilename(nullptr) {
	m_VAO[0] = 0;
	m_VAO[1] = 0;
	m_VBO[0] = 0;
	m_VBO[1] = 0;
}

GPUEmitter::~GPUEmitter(){
	delete[] m_particles;

	glDeleteVertexArrays(2, m_VAO);
	glDeleteBuffers(2, m_VBO);

	glDeleteProgram(m_instantRender);
	glDeleteProgram(m_deferredRender);
	glDeleteProgram(m_updateShader);
}

void GPUEmitter::Init(vec3 a_pos, vec3 a_extents, unsigned int a_maxParticles, float a_lifespanMin,
	float a_lifespanMax, float a_velocityMin, float a_velocityMax, float a_fadeIn, float a_fadeOut,
	float a_startSize, float a_endSize, vec4 a_startColor, vec4 a_endColor, 
	EmitType a_emitType, MoveType a_moveType, char* a_szFilename) {
	m_maxParticles = a_maxParticles;
	m_pos = a_pos;
	m_extents = a_extents;
	m_lifespanMin = a_lifespanMin;
	m_lifespanMax = a_lifespanMax;
	m_velocityMin = a_velocityMin;
	m_velocityMax = a_velocityMax;
	m_fadeIn = a_fadeIn;
	m_fadeOut = a_fadeOut;
	m_startSize = a_startSize;
	m_endSize = a_endSize;
	m_startColor = a_startColor;
	m_endColor = a_endColor;
	m_emitType = a_emitType;
	m_moveType = a_moveType;
	m_szFilename = a_szFilename;

	m_activeBuffer = 0;

	m_particles = new GPUParticle[m_maxParticles];

	CreateBuffers();
	CreateTexture();

	Reload();
}

void GPUEmitter::CreateBuffers() {
	glGenVertexArrays(2, m_VAO);
	glGenBuffers(2, m_VBO);

	for (unsigned int buffer_index = 0; buffer_index < 2; ++buffer_index)
	{
		glBindVertexArray(m_VAO[buffer_index]);
		glBindBuffer(GL_ARRAY_BUFFER, m_VBO[buffer_index]);
		glBufferData(GL_ARRAY_BUFFER, m_maxParticles * sizeof(GPUParticle), m_particles, GL_STREAM_DRAW);

		glEnableVertexAttribArray(0);//position
		glEnableVertexAttribArray(1);//velocity
		glEnableVertexAttribArray(2);//lifetime
		glEnableVertexAttribArray(3);//lifespan

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GPUParticle), 0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GPUParticle), (void*)12);
		glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(GPUParticle), (void*)24);
		glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(GPUParticle), (void*)28);
	}

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void GPUEmitter::CreateTexture() {
	std::string filename = m_szFilename;

	int width, height, channels;
	unsigned char* data = stbi_load(m_szFilename, &width, &height, &channels, STBI_default);

	glGenTextures(1, &m_texture);
	glBindTexture(GL_TEXTURE_2D, m_texture);

	glTexImage2D(GL_TEXTURE_2D, 0, channels, width, height, 0, (channels == 3) ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	stbi_image_free(data);
}

void GPUEmitter::Render(float a_currTime, FlyCamera a_camera, bool a_deferred) {
	glUseProgram(m_updateShader);

	int loc = glGetUniformLocation(m_updateShader, "time");
	glUniform1f(loc, a_currTime);

	loc = glGetUniformLocation(m_updateShader, "deltaTime");
	glUniform1f(loc, (a_currTime - m_lastDrawTime > 0.1) ? 0.1 : a_currTime - m_lastDrawTime);

	loc = glGetUniformLocation(m_updateShader, "emitPos");
	glUniform3fv(loc, 1, &m_pos[0]);

	loc = glGetUniformLocation(m_updateShader, "extents");
	glUniform3fv(loc, 1, &m_extents[0]);

	loc = glGetUniformLocation(m_updateShader, "minVel");
	glUniform1f(loc, m_velocityMin);

	loc = glGetUniformLocation(m_updateShader, "maxVel");
	glUniform1f(loc, m_velocityMax);

	loc = glGetUniformLocation(m_updateShader, "minLife");
	glUniform1f(loc, m_lifespanMin);

	loc = glGetUniformLocation(m_updateShader, "maxLife");
	glUniform1f(loc, m_lifespanMax);

	loc = glGetUniformLocation(m_updateShader, "emitType");
	glUniform1i(loc, (int)m_emitType);

	loc = glGetUniformLocation(m_updateShader, "moveType");
	glUniform1i(loc, (int)m_moveType);

	glEnable(GL_RASTERIZER_DISCARD);

	glBindVertexArray(m_VAO[m_activeBuffer]);
	unsigned int otherBuffer = (m_activeBuffer + 1) % 2;
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, m_VBO[otherBuffer]);
	glBeginTransformFeedback(GL_POINTS);
	glDrawArrays(GL_POINTS, 0, m_maxParticles);
	glEndTransformFeedback();

	glDisable(GL_RASTERIZER_DISCARD);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, 0);

	unsigned int renderer = (a_deferred) ? m_deferredRender : m_instantRender;
	glUseProgram(renderer);

	loc = glGetUniformLocation(renderer, "startSize");
	glUniform1f(loc, m_startSize);

	loc = glGetUniformLocation(renderer, "endSize");
	glUniform1f(loc, m_endSize);

	loc = glGetUniformLocation(renderer, "startColor");
	glUniform4fv(loc, 1, &m_startColor[0]);

	loc = glGetUniformLocation(renderer, "endColor");
	glUniform4fv(loc, 1, &m_endColor[0]);

	loc = glGetUniformLocation(renderer, "fadeIn");
	glUniform1f(loc, m_fadeIn);

	loc = glGetUniformLocation(renderer, "fadeOut");
	glUniform1f(loc, m_fadeOut);

	loc = glGetUniformLocation(renderer, "deferred");
	glUniform1i(loc, a_deferred);

	loc = glGetUniformLocation(renderer, "projView");
	glUniformMatrix4fv(loc, 1, false, &a_camera.getView()[0][0]);

	loc = glGetUniformLocation(renderer, "projView");
	glUniformMatrix4fv(loc, 1, false, &a_camera.getProjectionView()[0][0]);

	loc = glGetUniformLocation(renderer, "camTransform");
	glUniformMatrix4fv(loc, 1, false, &a_camera.getWorldTransform()[0][0]);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_texture);
	loc = glGetUniformLocation(renderer, "diffuse");
	glUniform1i(loc, 0);

	glBindVertexArray(m_VAO[otherBuffer]);
	glDrawArrays(GL_POINTS, 0, m_maxParticles);

	// swap for next frame
	m_activeBuffer = otherBuffer;
	m_lastDrawTime = a_currTime;
}

void GPUEmitter::Reload() {
	glDeleteProgram(m_instantRender);
	glDeleteProgram(m_updateShader);

	unsigned int vs;
	LoadShader("./data/shaders/gpuparticles_update_vertex.glsl", GL_VERTEX_SHADER, &vs);

	m_updateShader = glCreateProgram();
	glAttachShader(m_updateShader, vs);

	const char* varyings[] = { "vPosition", "vVelocity", "vLifetime", "vLifespan" };

	glTransformFeedbackVaryings(m_updateShader, 4, varyings, GL_INTERLEAVED_ATTRIBS);

	glLinkProgram(m_updateShader);
	glDeleteShader(vs);

	LoadShader("./data/shaders/gpuparticles_vertex.glsl", "./data/shaders/gpuparticles_geometry.glsl", "./data/shaders/gpuparticles_fragment.glsl", &m_instantRender);
	LoadShader("./data/shaders/gpuparticles_vertex.glsl", "./data/shaders/gpuparticles_geometry.glsl", "./data/shaders/gbuffer_textured_fragment.glsl", &m_deferredRender);
}

void GPUEmitter::DrawDebugGizmos(){
	Gizmos::addTransform(glm::translate(mat4(1), vec3(m_pos)), 1);
	switch (m_emitType) {
	case(EMIT_POINT) : {
		break;
	}
	case(EMIT_LINE) : {
		Gizmos::addLine(m_pos, m_pos + m_extents, m_startColor);
		break;
	}
	case(EMIT_PLANE) : {
		Gizmos::addAABB(m_pos, vec3(m_extents.x, 0, m_extents.z), m_startColor);
		break;
	}
	case(EMIT_RECTANGLE) : {
		Gizmos::addAABB(m_pos, m_extents, m_startColor);
		break;
	}
	case(EMIT_OUTER_RECTANGLE) : {
		Gizmos::addAABB(m_pos, m_extents, m_startColor);
		break;
	}
	case(EMIT_RING) : {
		Gizmos::addRing(m_pos, m_extents.x, m_extents.x - 0.05f, 32, m_startColor);
		break;
	}
	case(EMIT_OUTER_RING) : {
		Gizmos::addRing(m_pos, m_extents.x, m_extents.x - 0.05f, 32, m_startColor);
		break;
	}
	case(EMIT_SPHERE) : {
		Gizmos::addSphere(m_pos, m_extents.x, 16, 16, vec4(m_startColor.xyz, 0.1f));
		break;
	}
	case(EMIT_OUTER_SPHERE) : {
		Gizmos::addSphere(m_pos, m_extents.x, 16, 16, vec4(m_startColor.xyz, 0.1f));
		break;
	}
	}
}