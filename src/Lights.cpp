#include "Lights.h"
#include "stb_image.h"

//Projection used for rendering all directional lights.
const mat4 DirectionalLight::m_lightProj = glm::ortho<float>(-400, 400, -400, 400, -650, 650);
const mat4 PointLight::m_lightProj = glm::perspective<float>(90, 1, -650, 650);
const CameraDirection PointLight::m_cubemapDirection[6] = {
	{ GL_TEXTURE_CUBE_MAP_POSITIVE_X, vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f) },
	{ GL_TEXTURE_CUBE_MAP_NEGATIVE_X, vec3(-1.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f) },
	{ GL_TEXTURE_CUBE_MAP_POSITIVE_Y, vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f) },
	{ GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, vec3(0.0f, -1.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f) },
	{ GL_TEXTURE_CUBE_MAP_POSITIVE_Z, vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, -1.0f, 0.0f) },
	{ GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, vec3(0.0f, 0.0f, -1.0f), vec3(0.0f, -1.0f, 0.0f) }
};


PointLight::PointLight(const vec3 a_pos, const vec3 a_color, const float a_radius) : m_pos(a_pos), m_color(a_color), m_radius(a_radius) {
	//ShadowMap Framebuffer
	glGenFramebuffers(1, &m_shadowFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_shadowFBO);

	// Create the depth buffer
	glGenTextures(1, &m_depthTexture);
	glBindTexture(GL_TEXTURE_2D, m_depthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, 512, 512, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	// Create the cube map
	glGenTextures(1, &m_shadowMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_shadowMap);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	for (unsigned int i = 0; i < 6; i++) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_R32F, 512, 512, 0, GL_RED, GL_FLOAT, NULL);
	}

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthTexture, 0);

	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
		printf("Error creating point light buffer!\n");

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

PointLight::~PointLight() {
	glDeleteFramebuffers(1, &m_shadowFBO);
	glDeleteTextures(1, &m_shadowMap);
	glDeleteTextures(1, &m_depthTexture);
}

DirectionalLight::DirectionalLight(const vec3 a_dir, const vec3 a_color) : m_dir(glm::normalize(a_dir)), m_color(a_color) {
	//ShadowMap Framebuffer
	glGenFramebuffers(1, &m_shadowFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_shadowFBO);

	glGenTextures(1, &m_shadowMap);
	glBindTexture(GL_TEXTURE_2D, m_shadowMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 4096, 4096, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	//Define special parameter to force red channel to be compared to depth channel.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_shadowMap, 0);

	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
		printf("Error creating directional light buffer!\n");

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

DirectionalLight::~DirectionalLight() {
	glDeleteFramebuffers(1, &m_shadowFBO);
	glDeleteTextures(1, &m_shadowMap);
}


void DirectionalLight::Render(Camera* a_camera, const unsigned int a_program) {
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, m_shadowMap);

	mat4 offsetScale = mat4(
	0.5f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.5f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.5f, 0.0f,
	0.5f, 0.5f, 0.5f, 1.0f);

	mat4 lightView = glm::lookAt(vec3(0), m_dir, vec3(0, 1, 0));
	mat4 lightMatrix = m_lightProj * lightView * mat4(1);
	lightMatrix = offsetScale * lightMatrix;

	vec4 viewspaceLightDir = a_camera->GetView() * vec4(glm::normalize(m_dir), 0);

	//Assign uniforms
	int loc = glGetUniformLocation(a_program, "lightDir");
	glUniform3fv(loc, 1, (float*)&viewspaceLightDir);
	loc = glGetUniformLocation(a_program, "lightCol");
	glUniform3fv(loc, 1, (float*)&m_color);
	loc = glGetUniformLocation(a_program, "lightMatrix");
	glUniformMatrix4fv(loc, 1, GL_FALSE, (float*)&lightMatrix);
}

void PointLight::Render(Camera* a_camera, const unsigned int a_program) {
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_shadowMap);

	vec4 viewspaceLightPos = a_camera->GetView() * vec4(m_pos, 1);

	//Assign uniform locations outside of loop, for efficiency.
	int loc = glGetUniformLocation(a_program, "lightPos");
	glUniform3fv(loc, 1, (float*)&m_pos);
	loc = glGetUniformLocation(a_program, "lightViewPos");
	glUniform3fv(loc, 1, (float*)&viewspaceLightPos);
	loc = glGetUniformLocation(a_program, "lightRadius");
	glUniform1f(loc, m_radius);
	loc = glGetUniformLocation(a_program, "lightCol");
	glUniform3fv(loc, 1, (float*)&m_color);
}

Sprite::Sprite(const vec3 a_pos, const float a_size, const vec3 a_color, const char* a_filename) : m_timer(0) {
	Init(a_pos, a_size, a_size, a_color, a_color, a_filename);
}

Sprite::Sprite(const vec3 a_pos, const float a_maxSize, const float a_minSize, const vec3 a_maxColor, const vec3 a_minColor, const char* a_filename) : m_timer(0) {
	Init(a_pos, a_maxSize, a_minSize, a_maxColor, a_minColor, a_filename);
}

Sprite::~Sprite(){
	glDeleteVertexArrays(1, &m_buffers.m_VAO);
	glDeleteBuffers(1, &m_buffers.m_VBO);
	glDeleteBuffers(1, &m_buffers.m_IBO);
	glDeleteTextures(1, &m_texture);
	glDeleteProgram(m_shader);
}

void Sprite::Init(const vec3 a_pos, const float a_maxSize, const float a_minSize, const vec3 a_maxColor, const vec3 a_minColor, const char* a_filename) {
	m_pos = a_pos;

	m_maxSize = a_maxSize;
	m_minSize = a_minSize;
	m_maxColor = a_maxColor;
	m_minColor = a_minColor;

	m_indexData[0] = 0;
	m_indexData[1] = 1;
	m_indexData[2] = 2;

	m_indexData[3] = 0;
	m_indexData[4] = 2;
	m_indexData[5] = 3;

	//tex coords never change. define them in the initialisation.
	m_vertexData[0].texCoord = vec2(0, 1);
	m_vertexData[1].texCoord = vec2(0, 0);
	m_vertexData[2].texCoord = vec2(1, 0);
	m_vertexData[3].texCoord = vec2(1, 1);

	glGenVertexArrays(1, &m_buffers.m_VAO);
	glGenBuffers(1, &m_buffers.m_VBO);
	glGenBuffers(1, &m_buffers.m_IBO);

	glBindVertexArray(m_buffers.m_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, m_buffers.m_VBO);
	glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(VertexParticle), m_vertexData, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffers.m_IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), m_indexData, GL_STATIC_DRAW);

	//Attrib array numbers should match the way VertexParticle lays them out.
	glEnableVertexAttribArray(0); //Position
	glEnableVertexAttribArray(1); //TexCoord
	glEnableVertexAttribArray(2); //Color

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(VertexParticle), 0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(VertexParticle), (void*)sizeof(vec4));
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(VertexParticle), (void*)(sizeof(vec4) + sizeof(vec2)));

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	//Load texture in
	int width, height, channels;
	unsigned char* data = stbi_load(a_filename, &width, &height, &channels, STBI_default);

	glGenTextures(1, &m_texture);
	glBindTexture(GL_TEXTURE_2D, m_texture);

	glTexImage2D(GL_TEXTURE_2D, 0, channels, width, height, 0, (channels == 3) ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	stbi_image_free(data);

	LoadShader("./data/shaders/sprite.vs", 0, "./data/shaders/gbuffer_textured.fs", &m_shader);
}

void Sprite::Update(const float dt, const mat4 a_camTransform) {
	m_timer += dt;

	float t = fmod(m_frequency, m_timer);

	m_color = glm::mix(m_maxColor, m_minColor, t);
	m_size = glm::mix(m_maxSize, m_minSize, t);

	//billboarding for sprite
	vec3 f = glm::normalize(vec3(a_camTransform[3]) - m_pos);
	vec3 r = glm::cross(vec3(a_camTransform[1]), f);
	vec3 u = glm::cross(f, r);

	mat4 particleTransform;
	particleTransform[0].xyz = r * m_size;
	particleTransform[1].xyz = u * m_size;
	particleTransform[2].xyz = f * m_size;
	particleTransform[3] = vec4(m_pos, 1);

	m_vertexData[0].position = particleTransform * vec4(-1, 1, 0, 1);
	m_vertexData[1].position = particleTransform * vec4(-1, -1, 0, 1);
	m_vertexData[2].position = particleTransform * vec4(1, -1, 0, 1);
	m_vertexData[3].position = particleTransform * vec4(1, 1, 0, 1);

	m_vertexData[0].color = vec4(m_color, 1);
	m_vertexData[1].color = vec4(m_color, 1);
	m_vertexData[2].color = vec4(m_color, 1);
	m_vertexData[3].color = vec4(m_color, 1);
}

void Sprite::Render(Camera* a_camera) {
	//store current shader prior to using model shader
	int prevShader = 0;
	glGetIntegerv(GL_CURRENT_PROGRAM, &prevShader);
	glUseProgram(m_shader);

	int loc = glGetUniformLocation(m_shader, "projView");
	glUniformMatrix4fv(loc, 1, GL_FALSE, (float*)&a_camera->GetProjectionView());

	loc = glGetUniformLocation(m_shader, "view");
	glUniformMatrix4fv(loc, 1, GL_FALSE, (float*)&a_camera->GetView());

	loc = glGetUniformLocation(m_shader, "textureScale");
	glUniform1f(loc, 1.0f);

	loc = glGetUniformLocation(m_shader, "diffuse");
	glUniform1i(loc, 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_texture);

	//Update vertexData with new position / color values.
	glBindBuffer(GL_ARRAY_BUFFER, m_buffers.m_VBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 4 * sizeof(VertexParticle), m_vertexData);

	glBindVertexArray(m_buffers.m_VAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	glUseProgram(prevShader);
}