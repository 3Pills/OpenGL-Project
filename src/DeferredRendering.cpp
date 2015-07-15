#include "DeferredRendering.h"
#include "Utility.h"
#include "tiny_obj_loader.h"
#include "AntTweakBar.h"

DeferredRendering::DeferredRendering(): m_oCamera(50){
	Application::Application();
}
DeferredRendering::~DeferredRendering(){}

bool DeferredRendering::startup(){
	if (!Application::startup()){
		return false;
	}

	TwInit(TW_OPENGL_CORE, nullptr);
	TwWindowSize(m_iWidth, m_iHeight);

	glfwSetMouseButtonCallback(m_window, OnMouseButton);
	glfwSetCursorPosCallback(m_window, OnMousePosition);
	glfwSetScrollCallback(m_window, OnMouseScroll);

	glfwSetKeyCallback(m_window, OnKey);
	glfwSetCharCallback(m_window, OnChar);

	glfwSetWindowSizeCallback(m_window, OnWindowResize);

	m_oCamera.setPerspective(glm::radians(50.0f), 1280.0f / 720.0f, 0.1f, 20000.0f);

	BuildMesh();
	BuildQuad();
	BuildCube();

	BuildGBuffer();
	BuildLightBuffer();

	LoadShader("./data/shaders/gbuffer_vertex.glsl", 0, "./data/shaders/gbuffer_fragment.glsl", &m_gBufferProgram);
	LoadShader("./data/shaders/composite_vertex.glsl", 0, "./data/shaders/composite_fragment.glsl", &m_compositeProgram);
	LoadShader("./data/shaders/composite_vertex.glsl", 0, "./data/shaders/directional_light_fragment.glsl", &m_directionalLightProgram);
	LoadShader("./data/shaders/point_light_vertex.glsl", 0, "./data/shaders/point_light_fragment.glsl", &m_pointLightProgram);

	TwBar* m_bar = TwNewBar("Settings");
	//TwAddVarRW(m_bar, "Draw Albedo", TW_TYPE_COLOR3F &m_bLi, "");

	Gizmos::create();
	return true;
}
bool DeferredRendering::shutdown(){
	Gizmos::destroy();
	return Application::shutdown();
}
bool DeferredRendering::update(){
	if (!Application::update()){
		return false;
	}
	m_oCamera.update(m_fDeltaTime);
	Gizmos::clear();
	Gizmos::addTransform(mat4(1), 10);

	vec4 white(1);
	vec4 black(0, 0, 0, 1);

	for (int i = 0; i <= 20; ++i){
		Gizmos::addLine(vec3(-10 + i, 0, -10), i != 10 ? vec3(-10 + i, 0, 10) : vec3(-10 + i, 0, 0), i != 10 ? black : white);
		Gizmos::addLine(vec3(-10, 0, -10 + i), i != 10 ? vec3(10, 0, -10 + i) : vec3(0, 0, -10 + i), i != 10 ? black : white);
	}
	if (glfwGetKey(m_window, GLFW_KEY_R) == GLFW_PRESS){
		ReloadShader();
	}

	return true;
}
void DeferredRendering::draw(){
	//Depth Rendering
	glEnable(GL_DEPTH_TEST);

	glBindFramebuffer(GL_FRAMEBUFFER, m_gBufferFBO);
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	vec4 clearColor = vec4(0, 0, 0, 0);
	vec4 clearNormal = vec4(0.5f, 0.5f, 0.5f, 0.5f);

	glClearBufferfv(GL_COLOR, 0, (float*)&clearColor);
	glClearBufferfv(GL_COLOR, 1, (float*)&clearColor);
	glClearBufferfv(GL_COLOR, 2, (float*)&clearNormal);

	glUseProgram(m_gBufferProgram);

	int view_uniform = glGetUniformLocation(m_gBufferProgram, "view");
	int projView_uniform = glGetUniformLocation(m_gBufferProgram, "projView");

	if (view_uniform > -1)
		glUniformMatrix4fv(view_uniform, 1, GL_FALSE, (float*)&m_oCamera.getView());
	if (projView_uniform > -1)
		glUniformMatrix4fv(projView_uniform, 1, GL_FALSE, (float*)&m_oCamera.getProjectionView());

	glBindVertexArray(m_bunny.m_VAO);
	glDrawElements(GL_TRIANGLES, m_bunny.m_indexCount, GL_UNSIGNED_INT, 0);

	//Light Rendering
	glBindFramebuffer(GL_FRAMEBUFFER, m_lightFBO);
	glClear(GL_COLOR_BUFFER_BIT);

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	//glUseProgram(m_directionalLightProgram);
	//
	//int positionTexture_uniform = glGetUniformLocation(m_directionalLightProgram, "positionTexture");
	//int normalTexture_uniform = glGetUniformLocation(m_directionalLightProgram, "normalTexture");
	//
	//if (positionTexture_uniform > -1)
	//	glUniform1i(positionTexture_uniform, 0);
	//if (normalTexture_uniform > -1)
	//	glUniform1i(normalTexture_uniform, 1);
	//
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, m_positionTexture);
	//glActiveTexture(GL_TEXTURE1);
	//glBindTexture(GL_TEXTURE_2D, m_normalTexture);
	//
	//RenderDirectionalLight(vec3(1, 0, 0), vec3(1, 0, 0));
	//RenderDirectionalLight(vec3(0, 1, 0), vec3(0, 1, 0));
	//RenderDirectionalLight(vec3(0, 0, 1), vec3(0, 0, 1));

	glUseProgram(m_pointLightProgram);

	projView_uniform = glGetUniformLocation(m_pointLightProgram, "projView");
	int positionTexture_uniform = glGetUniformLocation(m_pointLightProgram, "positionTexture");
	int normalTexture_uniform = glGetUniformLocation(m_pointLightProgram, "normalTexture");

	if (projView_uniform > -1)
		glUniformMatrix4fv(projView_uniform, 1, GL_FALSE, (float*)&m_oCamera.getProjectionView());
	if (positionTexture_uniform > -1)
		glUniform1i(positionTexture_uniform, 0);
	if (normalTexture_uniform > -1)
		glUniform1i(normalTexture_uniform, 1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_positionTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_normalTexture);


	glCullFace(GL_FRONT);
	RenderPointLight(vec3(0, 20, 0), 500, vec3(1));
	//draw the point lights
	//for (int i = 0; i < 500; i++) {
	//	RenderPointLight(vec3(i / 2, i * 2, i % 4), 500, vec3((float)(i % 2) / 50, (float)((i + 1) % 2) / 50, 0));
	//}
	glCullFace(GL_BACK);

	glDisable(GL_BLEND);

	//Composite Rendering
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(0.3, 0.3, 0.3, 1);

	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(m_compositeProgram);

	int albedoTexture_uniform = glGetUniformLocation(m_compositeProgram, "albedoTexture");
	int lightTexture_uniform = glGetUniformLocation(m_compositeProgram, "lightTexture");

	if (albedoTexture_uniform > -1)
		glUniform1i(albedoTexture_uniform, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_albedoTexture);

	if (lightTexture_uniform > -1)
		glUniform1i(lightTexture_uniform, 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_lightTexture);

	glBindVertexArray(m_screenspaceQuad.m_VAO);
	glDrawElements(GL_TRIANGLES, m_screenspaceQuad.m_indexCount, GL_UNSIGNED_INT, 0);

	glEnable(GL_DEPTH_TEST);

	//Gizmos::draw(m_oCamera.getProjectionView());
	TwDraw();
	Application::draw();
}

void DeferredRendering::BuildGBuffer() {
	glGenFramebuffers(1, &m_gBufferFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_gBufferFBO);

	glGenTextures(1, &m_albedoTexture);
	glBindTexture(GL_TEXTURE_2D, m_albedoTexture);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB8, m_iWidth, m_iHeight);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glGenTextures(1, &m_positionTexture);
	glBindTexture(GL_TEXTURE_2D, m_positionTexture);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB32F, m_iWidth, m_iHeight);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glGenTextures(1, &m_normalTexture);
	glBindTexture(GL_TEXTURE_2D, m_normalTexture);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB32F, m_iWidth, m_iHeight);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glGenRenderbuffers(1, &m_depthTexture);
	glBindRenderbuffer(GL_RENDERBUFFER, m_depthTexture);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_iWidth, m_iHeight);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_albedoTexture, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, m_positionTexture, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, m_normalTexture, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthTexture);

	GLenum gPassTargets[] = { GL_COLOR_ATTACHMENT0,	GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, gPassTargets);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
		printf("Error creating gbuffer!\n");

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DeferredRendering::BuildLightBuffer() {
	glGenFramebuffers(1, &m_lightFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_lightFBO);

	glGenTextures(1, &m_lightTexture);
	glBindTexture(GL_TEXTURE_2D, m_lightTexture);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB8, m_iWidth, m_iHeight);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_lightTexture, 0);

	GLenum lightTargets = GL_COLOR_ATTACHMENT0;
	glDrawBuffers(1, &lightTargets);

	unsigned int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
		printf("Error creating light framebuffer!\n");

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DeferredRendering::BuildMesh()
{
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	tinyobj::LoadObj(shapes, materials, "./data/models/stanford/bunny.obj");

	m_bunny.m_indexCount = shapes[0].mesh.indices.size();

	tinyobj::mesh_t* mesh = &shapes[0].mesh;

	std::vector<float> vertexData;
	vertexData.reserve(mesh->positions.size() + mesh->normals.size());

	vertexData.insert(vertexData.end(), mesh->positions.begin(), mesh->positions.end());
	vertexData.insert(vertexData.end(), mesh->normals.begin(), mesh->normals.end());

	glGenVertexArrays(1, &m_bunny.m_VAO);
	glBindVertexArray(m_bunny.m_VAO);

	glGenBuffers(1, &m_bunny.m_VBO);
	glGenBuffers(1, &m_bunny.m_IBO);

	glBindBuffer(GL_ARRAY_BUFFER, m_bunny.m_VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bunny.m_IBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* vertexData.size(), vertexData.data(), GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)* mesh->indices.size(), mesh->indices.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(float)* mesh->positions.size()));

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void DeferredRendering::BuildQuad()
{
	vec2 half_texel = 1.0f / vec2(m_iWidth, m_iHeight) * 0.5f;

	float vertexData[]
	{
		-1, -1, 0, 1,	  half_texel.x,		half_texel.y,
		 1, -1, 0, 1, 1 - half_texel.x,		half_texel.y,
		 1,  1, 0, 1, 1 - half_texel.x, 1 - half_texel.y,
		-1,  1, 0, 1,	  half_texel.x,	1 - half_texel.y,
	};

	unsigned int indexData[] = { 0, 1, 2, 0, 2, 3 };

	m_screenspaceQuad.m_indexCount = 6;

	glGenVertexArrays(1, &m_screenspaceQuad.m_VAO);

	glGenBuffers(1, &m_screenspaceQuad.m_VBO);
	glGenBuffers(1, &m_screenspaceQuad.m_IBO);

	glBindVertexArray(m_screenspaceQuad.m_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, m_screenspaceQuad.m_VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_screenspaceQuad.m_IBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexData), indexData, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(float)* 6, 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float)* 6, (void*)(sizeof(float)* 4));

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void DeferredRendering::BuildCube() {
	//vec2 half_texel = 1.0f / vec2(m_iWidth, m_iHeight) * 0.5f;

	float vertexData[]
	{
		-1, -1,  1,  1,
		 1, -1,  1,  1,
		 1, -1, -1,  1, 
		-1, -1, -1,  1,
					 
		-1,  1,  1,  1,
		 1,  1,  1,  1,
		 1,  1, -1,  1,
		-1,  1, -1,  1
	};

	unsigned int indexData[] = {
		4, 5, 0,
		5, 1, 0,
		5, 6, 1,
		6, 2, 1,
		6, 7, 2,
		7, 3, 2,
		7, 4, 3,
		4, 0, 3,
		7, 6, 4,
		6, 5, 4,
		0, 1, 3,
		1, 2, 3
	};

	m_lightCube.m_indexCount = 36;

	glGenVertexArrays(1, &m_lightCube.m_VAO);
	glBindVertexArray(m_lightCube.m_VAO);

	glGenBuffers(1, &m_lightCube.m_VBO);
	glGenBuffers(1, &m_lightCube.m_IBO);


	glBindBuffer(GL_ARRAY_BUFFER, m_lightCube.m_VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_lightCube.m_IBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexData), indexData, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(float)* 4, 0);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void DeferredRendering::RenderDirectionalLight(vec3 a_lightDir, vec3 a_lightColor)
{
	vec4 viewspaceLightDir = m_oCamera.getView() * vec4(glm::normalize(a_lightDir), 0);

	int lightDir_uniform = glGetUniformLocation(m_directionalLightProgram, "lightDir");
	int lightColor_uniform = glGetUniformLocation(m_directionalLightProgram, "lightColor");

	glUniform3fv(lightDir_uniform, 1, (float*)&viewspaceLightDir);
	glUniform3fv(lightColor_uniform, 1, (float*)&a_lightColor);

	glBindVertexArray(m_screenspaceQuad.m_VAO);
	glDrawElements(GL_TRIANGLES, m_screenspaceQuad.m_indexCount, GL_UNSIGNED_INT, 0);
}

void DeferredRendering::RenderPointLight(vec3 a_lightPos, float a_radius, vec3 a_lightColor){
	vec4 viewspaceLightPos = m_oCamera.getView() * vec4(a_lightPos, 1);

	int lightPos_uniform = glGetUniformLocation(m_pointLightProgram, "lightPos");
	int lightViewPos_uniform = glGetUniformLocation(m_pointLightProgram, "lightViewPos");
	int lightRadius_uniform = glGetUniformLocation(m_pointLightProgram, "lightRadius");
	int lightColor_uniform = glGetUniformLocation(m_pointLightProgram, "lightColor");

	glUniform3fv(lightPos_uniform, 1, (float*)&a_lightPos);
	glUniform3fv(lightViewPos_uniform, 1, (float*)&viewspaceLightPos);
	glUniform3fv(lightColor_uniform, 1, (float*)&a_lightColor);
	glUniform1f(lightRadius_uniform, a_radius);

	glBindVertexArray(m_lightCube.m_VAO);
	glDrawElements(GL_TRIANGLES, m_lightCube.m_indexCount, GL_UNSIGNED_INT, 0);
}

void DeferredRendering::ReloadShader(){
	glDeleteProgram(m_gBufferProgram);
	LoadShader("./data/shaders/gbuffer_vertex.glsl", 0, "./data/shaders/gbuffer_fragment.glsl", &m_gBufferProgram);
	LoadShader("./data/shaders/composite_vertex.glsl", 0, "./data/shaders/composite_fragment.glsl", &m_compositeProgram);
	LoadShader("./data/shaders/composite_vertex.glsl", 0, "./data/shaders/directional_light_fragment.glsl", &m_directionalLightProgram);
	LoadShader("./data/shaders/point_light_vertex.glsl", 0, "./data/shaders/point_light_fragment.glsl", &m_pointLightProgram);
}