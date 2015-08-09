#include "RenderTargets.h"
#include "Utility.h"

RenderTargets::RenderTargets(): m_oCamera(50){
	Application::Application();
}
RenderTargets::~RenderTargets(){}

bool RenderTargets::startup(){
	if (!Application::startup()){
		return false;
	}

	m_oCamera.SetPerspective(glm::radians(50.0f), 1280.0f / 720.0f, 0.1f, 20000.0f);

	GenerateFramebuffer();
	GeneratePlane();
	LoadShader("./data/shaders/reflective.vs", 0, "./data/shaders/reflective.fs", &m_programID);

	Gizmos::create();
	return true;
}

bool RenderTargets::shutdown(){
	return Application::shutdown();
}

bool RenderTargets::update(){
	if (!Application::update()){
		return false;
	}
	m_oCamera.Update(m_fDeltaTime);

	return true;
}

int reflectionResolution = 1024;

void RenderTargets::draw(){
	Gizmos::clear();
	Gizmos::addTransform(mat4(1), 10);
	Gizmos::addSphere(vec3(0, 5, 0), 0.5f, 12, 12, vec4(1, 1, 0, 1));

	vec4 white(1);
	vec4 black(0, 0, 0, 1);

	for (int i = 0; i <= 20; ++i){
		Gizmos::addLine(vec3(-10 + i, 0, -10), i != 10 ? vec3(-10 + i, 0, 10) : vec3(-10 + i, 0, 0), i != 10 ? black : white);
		Gizmos::addLine(vec3(-10, 0, -10 + i), i != 10 ? vec3(10, 0, -10 + i) : vec3(0, 0, -10 + i), i != 10 ? black : white);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	glViewport(0, 0, reflectionResolution, reflectionResolution);
	glClearColor(0.75f, 0.75f, 0.75f, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	mat4 world = m_oCamera.GetWorldTransform();

	vec4 plane = vec4(0, 0, 1, -5);
	vec3 reflected = glm::reflect(-world[2].xyz(), plane.xyz());
	reflected = glm::normalize(reflected);

	float dist = (glm::dot((plane.xyz() * plane.w) - world[3].xyz(), plane.xyz())) / 
				 (glm::dot( plane.xyz(), -world[2].xyz()));

	vec3 inter = world[3].xyz() + (-world[2].xyz()) * dist;
	vec3 up = vec3(0, 1, 0);

	world[3].xyz = inter - reflected * dist;
	world[2].xyz = -reflected;
	world[0].xyz = glm::normalize(glm::cross(world[2].xyz(), up));
	world[1].xyz = glm::normalize(glm::cross(world[0].xyz(), world[2].xyz()));


	mat4 view = glm::inverse(world);
	mat4 projView = m_oCamera.GetProjection() * view;

	glUseProgram(m_programID);

	int view_proj_uniform = glGetUniformLocation(m_programID, "projView");
	if (view_proj_uniform > -1)
		glUniformMatrix4fv(view_proj_uniform, 1, GL_FALSE, (float*)&m_oCamera.GetProjectionView());

	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	glBindVertexArray(m_plane.m_VAO);
	glDrawElements(GL_TRIANGLES, m_plane.m_indexCount, GL_UNSIGNED_INT, nullptr);


	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	glDepthFunc(GL_GREATER);
	Gizmos::draw(projView);
	glDepthFunc(GL_LESS); 

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, m_iWidth, m_iHeight);

	glClearColor(0.3f, 0.3f, 0.3f, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Gizmos::draw(m_oCamera.GetProjectionView());

	int view_proj_reflected_uniform = glGetUniformLocation(m_programID, "projViewReflected");
	if (view_proj_reflected_uniform > -1)
		glUniformMatrix4fv(view_proj_reflected_uniform, 1, GL_FALSE, (float*)&projView);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_fboTexture);

	int diff_uniform = glGetUniformLocation(m_programID, "diffuse");
	if (diff_uniform > -1)
		glUniform1i(diff_uniform, 0);

	glBindVertexArray(m_plane.m_VAO);
	glDrawElements(GL_TRIANGLES, m_plane.m_indexCount, GL_UNSIGNED_INT, nullptr);

	Application::draw();
}

void RenderTargets::GenerateFramebuffer(){
	glGenFramebuffers(1, &m_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

	glGenTextures(1, &m_fboTexture);
	glBindTexture(GL_TEXTURE_2D, m_fboTexture);
	glTexStorage2D(GL_TEXTURE_2D, 7, GL_RGB8, reflectionResolution, reflectionResolution);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_fboTexture, 0);

	glGenRenderbuffers(1, &m_fboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, m_fboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, reflectionResolution, reflectionResolution);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_fboDepth);

	GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, drawBuffers);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) printf("Framebuffer failed to generate!\n");
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderTargets::GeneratePlane(){
	float vertexData[] = {
		-5,  0, -5, 1, 0, 0,
		 5,  0, -5, 1, 1, 0,
		 5, 10, -5, 1, 1, 1,
		-5, 10, -5, 1, 0, 1,
	};

	unsigned int indexData[] = {
		0, 1, 2,
		0, 2, 3,
	};

	m_plane.m_indexCount = sizeof(indexData);

	glGenVertexArrays(1, &m_plane.m_VAO);
	glBindVertexArray(m_plane.m_VAO);

	glGenBuffers(1, &m_plane.m_VBO);
	glGenBuffers(1, &m_plane.m_IBO);

	glBindBuffer(GL_ARRAY_BUFFER, m_plane.m_VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_plane.m_IBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexData), indexData, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0); //position
	glEnableVertexAttribArray(1); //texcoord

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(indexData), 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(indexData), (void*)(sizeof(float)* 4));

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}