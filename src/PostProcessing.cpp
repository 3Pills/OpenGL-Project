#include "PostProcessing.h"
#include "Utility.h"

bool shouldResize = false;

PostProcessing::PostProcessing(): m_oCamera(50){
	Application::Application();
}
PostProcessing::~PostProcessing(){}

bool PostProcessing::startup(){
	if (!Application::startup()){
		return false;
	}

	m_oCamera.setPerspective(glm::radians(50.0f), 1280.0f / 720.0f, 0.1f, 20000.0f);

	GenerateFramebuffer();
	GenerateQuad();

	LoadShader("./data/shaders/post_vertex.glsl", 0, "./data/shaders/post_fragment.glsl", &m_postProgramID);

	Gizmos::create();
	return true;
}
bool PostProcessing::shutdown(){
	return Application::shutdown();
}
bool PostProcessing::update(){
	if (!Application::update()){
		return false;
	}
	m_oCamera.update(m_fDeltaTime);
	if (glfwGetKey(m_window, GLFW_KEY_R) == GLFW_PRESS && m_LastKey != GLFW_PRESS){
		ReloadShader();
	}
	m_LastKey = glfwGetKey(m_window, GLFW_KEY_R);

	return true;
}
void PostProcessing::draw(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	Gizmos::clear();
	Gizmos::addTransform(mat4(1), 10);

	vec4 white(1);
	vec4 black(0, 0, 0, 1);

	for (int i = 0; i <= 20; ++i){
		Gizmos::addLine(vec3(-10 + i, 0, -10), i != 10 ? vec3(-10 + i, 0, 10) : vec3(-10 + i, 0, 0), i != 10 ? black : white);
		Gizmos::addLine(vec3(-10, 0, -10 + i), i != 10 ? vec3(10, 0, -10 + i) : vec3(0, 0, -10 + i), i != 10 ? black : white);
	}
	
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	glViewport(0, 0, m_iWidth, m_iHeight);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Gizmos::draw(m_oCamera.getProjectionView());

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, m_iWidth, m_iHeight);

	glClear(GL_DEPTH_BUFFER_BIT);

	glUseProgram(m_postProgramID);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_fboTexture);


	int target_uniform = glGetUniformLocation(m_postProgramID, "target");
	if (target_uniform > -1) {
		glUniform1i(target_uniform, 0);
	}
	int time_uniform = glGetUniformLocation(m_postProgramID, "time");
	if (time_uniform > -1) {
		glUniform1f(time_uniform, m_fCurrTime);
	}

	int screenWidth_uniform = glGetUniformLocation(m_postProgramID, "screenWidth");
	if (screenWidth_uniform > -1) {
		glUniform1i(screenWidth_uniform, m_iWidth);
	}
	int screenHeight_uniform = glGetUniformLocation(m_postProgramID, "screenHeight");
	if (screenHeight_uniform > -1) {
		glUniform1i(screenHeight_uniform, m_iHeight);
	}

	glBindVertexArray(m_quad.m_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	Application::draw();
}

void PostProcessing::resize(int a_width, int a_height){
	Application::resize(a_width, a_height);
	GenerateFramebuffer();
	GenerateQuad();
}

void PostProcessing::GenerateFramebuffer() {
	glGenFramebuffers(1, &m_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

	glGenTextures(1, &m_fboTexture);
	glBindTexture(GL_TEXTURE_2D, m_fboTexture);
	glTexStorage2D(GL_TEXTURE_2D, 7, GL_RGB8, m_iWidth, m_iHeight);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_fboTexture, 0);

	glGenRenderbuffers(1, &m_fboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, m_fboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, m_iWidth, m_iHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_fboDepth);

	GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, drawBuffers);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) printf("Framebuffer failed to generate!\n");
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PostProcessing::GenerateQuad(){
	vec2 halfTexel = 1.0f / glm::vec2(m_iWidth, m_iHeight) * 0.5f;
	float vertexData[] = {
		-1, -1,  0,  1,		 halfTexel.x,	  halfTexel.y,
		 1,  1,  0,  1,  1 - halfTexel.x, 1 - halfTexel.y,
		-1,  1,  0,  1,		 halfTexel.x, 1 - halfTexel.y,
		-1, -1,  0,  1,		 halfTexel.x,	  halfTexel.y,
		 1, -1,  0,  1,  1 - halfTexel.x,	  halfTexel.y,
		 1,  1,  0,  1,  1 - halfTexel.x, 1 - halfTexel.y,
	};

	unsigned int indexData[] = {
		0, 1, 2, 0, 2, 3
	};

	m_quad.m_indexCount = 6;

	glGenVertexArrays(1, &m_quad.m_VAO);
	glBindVertexArray(m_quad.m_VAO);

	glGenBuffers(1, &m_quad.m_VBO);
	glGenBuffers(1, &m_quad.m_IBO);

	glBindBuffer(GL_ARRAY_BUFFER, m_quad.m_VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_quad.m_IBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexData), indexData, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(float)* 6, 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float)* 6, (void*)(sizeof(float) * 4));

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void PostProcessing::ReloadShader(){
	glDeleteProgram(m_postProgramID);
	LoadShader("./data/shaders/post_vertex.glsl", 0, "./data/shaders/post_fragment.glsl", &m_postProgramID);
}