#include "GPUEmitter.h"
#include "gl_core_4_4.h"
#include "stb_image.h"
#include <string>

GPUEmitter::GPUEmitter() :
	m_particles(nullptr), m_maxParticles(0), m_pos(0),
	m_emitRate(0), m_emitTimer(0), m_lifespanMin(0), m_lifespanMax(0),
	m_velocityMin(0), m_velocityMax(0), m_startSize(0), m_endSize(0),
	m_startColor(0), m_endColor(0), m_drawShader(0), m_updateShader(0),
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

	glDeleteProgram(m_drawShader);
	glDeleteProgram(m_updateShader);
}

void GPUEmitter::Init(vec3 a_pos, unsigned int a_maxParticles, float a_emitRate, float a_lifespanMin,
	float a_lifespanMax, float a_velocityMin, float a_velocityMax,
	float a_startSize, float a_endSize, vec4 a_startColor, vec4 a_endColor, EmitType a_emitType, char* a_szFilename) {
	m_maxParticles = a_maxParticles;
	m_pos = vec4(a_pos, 1);
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
	m_szFilename = a_szFilename;

	m_activeBuffer = 0;

	m_particles = new GPUParticle[m_maxParticles];

	CreateBuffers();
	CreateUpdateShader();
	CreateDrawShader();
	CreateTexture();
}

void GPUEmitter::CreateBuffers() {
	// create opengl buffers
	glGenVertexArrays(2, m_VAO);
	glGenBuffers(2, m_VBO);
	// setup the first buffer
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

void GPUEmitter::CreateDrawShader() {
	LoadShader("./shaders/gpuparticles_vertex.glsl", "./shaders/gpuparticles_geometry.glsl", "./shaders/gpuparticles_fragment.glsl", &m_drawShader);

	// bind the shader so that we can set
	// some uniforms that don't change per-frame
	glUseProgram(m_drawShader);

	// bind size information for interpolation that won’t change
	int location = glGetUniformLocation(m_drawShader, "sizeStart");
	if (location > -1)
		glUniform1f(location, m_startSize);
	location = glGetUniformLocation(m_drawShader, "sizeEnd");
	if (location > -1)
		glUniform1f(location, m_endSize);

	// bind colour information for interpolation that wont change
	location = glGetUniformLocation(m_drawShader, "colorStart");
	if (location > -1)
		glUniform4fv(location, 1, &m_startColor[0]);
	location = glGetUniformLocation(m_drawShader, "colorEnd");
	if (location > -1)
		glUniform4fv(location, 1, &m_endColor[0]);
}

void GPUEmitter::CreateUpdateShader() {
	// create a shader
	unsigned int vs;
	LoadShader("./shaders/gpuparticles_update_vertex.glsl", GL_VERTEX_SHADER, &vs);

	m_updateShader = glCreateProgram();
	glAttachShader(m_updateShader, vs);

	// specify the data that we will stream back
	const char* varyings[] = { "position", "velocity", "lifetime", "lifespan" };

	glTransformFeedbackVaryings(m_updateShader, 4, varyings, GL_INTERLEAVED_ATTRIBS);

	glLinkProgram(m_updateShader);

	// remove unneeded handles
	glDeleteShader(vs);

	// bind the shader so that we can set some
	// uniforms that don't change per-frame
	glUseProgram(m_updateShader);

	// bind lifetime minimum and maximum
	int location = glGetUniformLocation(m_updateShader, "lifeMin");
	if (location > -1)
		glUniform1f(location, m_lifespanMin);
	location = glGetUniformLocation(m_updateShader, "lifeMax");
	if (location > -1)
		glUniform1f(location, m_lifespanMax);

	int min_vel_uniform = glGetUniformLocation(m_updateShader, "min_velocity");
	glUniform1f(min_vel_uniform, m_velocityMin);

	int max_vel_uniform = glGetUniformLocation(m_updateShader, "max_velocity");
	glUniform1f(max_vel_uniform, m_velocityMax);

	int min_life_uniform = glGetUniformLocation(m_updateShader, "min_lifespan");
	glUniform1f(min_life_uniform, m_lifespanMin);

	int max_life_uniform = glGetUniformLocation(m_updateShader, "max_lifespan");
	glUniform1f(max_life_uniform, m_lifespanMax);

	int size_start_uniform = glGetUniformLocation(m_drawShader, "size_start");
	glUniform1f(size_start_uniform, m_startSize);

	int size_end_uniform = glGetUniformLocation(m_drawShader, "size_end");
	glUniform1f(size_end_uniform, m_endSize);

	int color_start_uniform = glGetUniformLocation(m_drawShader, "color_start");
	glUniform4fv(color_start_uniform, 1, (float*)&m_startColor);

	int color_end_uniform = glGetUniformLocation(m_drawShader, "color_end");
	glUniform4fv(color_end_uniform, 1, (float*)&m_endColor);
}

void GPUEmitter::CreateTexture() {
	std::string filename = m_szFilename;

	int width, height, channels;
	unsigned char* data = stbi_load(m_szFilename, &width, &height, &channels, STBI_default);

	glGenTextures(1, &m_texture);
	glBindTexture(GL_TEXTURE_2D, m_texture);

	const char* extension = filename.substr(filename.find_last_of("."), +1).c_str();
	if (extension == ".jpg") {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	}
	else {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	stbi_image_free(data);
}

void GPUEmitter::Render(float a_currTime, mat4 a_camTransform, mat4 a_projView) {
	glUseProgram(m_updateShader);

	// bind time information
	int time_uniform = glGetUniformLocation(m_updateShader, "time");
	glUniform1f(time_uniform, a_currTime);

	int delta_uniform = glGetUniformLocation(m_updateShader, "deltaTime");
	glUniform1f(delta_uniform, a_currTime - m_lastDrawTime);

	int emitterPos_uniform = glGetUniformLocation(m_updateShader, "emitterPos");
	glUniform3fv(emitterPos_uniform, 1, &m_pos[0]);

	glEnable(GL_RASTERIZER_DISCARD);

	// bind the buffer we will update
	glBindVertexArray(m_VAO[m_activeBuffer]);

	// work out the "other" buffer
	unsigned int otherBuffer = (m_activeBuffer + 1) % 2;

	// bind the buffer we will update into as points
	// and begin transform feedback
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, m_VBO[otherBuffer]);

	glBeginTransformFeedback(GL_POINTS);
	glDrawArrays(GL_POINTS, 0, m_maxParticles);
	glEndTransformFeedback();

	glDisable(GL_RASTERIZER_DISCARD);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, 0);

	glUseProgram(m_drawShader);
	int projView_uniform = glGetUniformLocation(m_drawShader, "projView");
	glUniformMatrix4fv(projView_uniform, 1, false, &a_projView[0][0]);

	int camTransform_uniform = glGetUniformLocation(m_drawShader, "camTransform");
	glUniformMatrix4fv(camTransform_uniform, 1, false, &a_camTransform[0][0]);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_texture);

	int diffuse_location = glGetUniformLocation(m_drawShader, "diffuse");
	glUniform1i(diffuse_location, 0);

	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glBindVertexArray(m_VAO[otherBuffer]);
	glDrawArrays(GL_POINTS, 0, m_maxParticles);


	// swap for next frame
	m_activeBuffer = otherBuffer;
	m_lastDrawTime = a_currTime;
}