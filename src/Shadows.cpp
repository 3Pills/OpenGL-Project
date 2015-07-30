#include "Shadows.h"
#include "tiny_obj_loader.h"
#include "Utility.h"

Shadows::Shadows(): m_oCamera(50){
	Application::Application();
}
Shadows::~Shadows(){}

bool Shadows::startup(){
	if (!Application::startup()){
		return false;
	}

	m_oCamera.setPerspective(glm::radians(50.0f), 1280.0f / 720.0f, 0.1f, 20000.0f);

	Gizmos::create();

	LoadShader("./data/shaders/diffuse_shadowed_vertex.glsl", 0, "./data/shaders/diffuse_shadowed_fragment.glsl", &m_diffuseShadowProgramID);
	LoadShader("./data/shaders/shadowed_vertex.glsl", 0, "./data/shaders/shadowed_fragment.glsl", &m_shadowProgramID);

	BuildMeshes();
	BuildShadowMap();

	m_lightDir = glm::normalize(glm::vec3(1, 2.5f, 1));

	mat4 lightProj = glm::ortho(-10, 10, -10, 10, -10, 10);
	mat4 lightView = glm::lookAt(m_lightDir, vec3(0), vec3(0, 1, 0));
	m_lightMatrix = lightProj * lightView;

	return true;
}

bool Shadows::shutdown(){
	return Application::shutdown();
}
bool Shadows::update(){
	if (!Application::update()){
		return false;
	}
	m_oCamera.update(m_fDeltaTime);
	if (glfwGetKey(m_window, GLFW_KEY_R) == GLFW_PRESS){
		ReloadShader();
	}

	return true;
}
void Shadows::draw(){
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	
	glViewport(0, 0, 1024, 1024);
	glClear(GL_DEPTH_BUFFER_BIT);

	glUseProgram(m_shadowProgramID);

	int lightMatrix_uniform = glGetUniformLocation(m_shadowProgramID, "lightMatrix");
	glUniformMatrix4fv(lightMatrix_uniform, 1, GL_FALSE, (float*)&m_lightMatrix);

	glBindVertexArray(m_bunny.m_VAO);
	glDrawElements(GL_TRIANGLES, m_bunny.m_indexCount, GL_UNSIGNED_INT, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, m_iWidth, m_iHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(m_diffuseShadowProgramID);

	int projView_uniform = glGetUniformLocation(m_diffuseShadowProgramID, "projView");
	glUniformMatrix4fv(projView_uniform, 1, GL_FALSE, (float*)&m_oCamera.getProjectionView());

	mat4 offsetScale = mat4(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.5f, 0.0f,
		0.5f, 0.5f, 0.5f, 1.0f);

	mat4 lightMatrix = offsetScale * m_lightMatrix;

	lightMatrix_uniform = glGetUniformLocation(m_diffuseShadowProgramID, "lightMatrix");
	glUniformMatrix4fv(lightMatrix_uniform, 1, GL_FALSE, (float*)&lightMatrix);

	int lightDir_uniform = glGetUniformLocation(m_diffuseShadowProgramID, "lightDir");
	glUniform3fv(lightDir_uniform, 1, &m_lightDir[0]);

	int shadowMap_uniform = glGetUniformLocation(m_diffuseShadowProgramID, "shadowMap");
	glUniform1i(shadowMap_uniform, 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_fboDepth);

	glBindVertexArray(m_bunny.m_VAO);
	glDrawElements(GL_TRIANGLES, m_bunny.m_indexCount, GL_UNSIGNED_INT, 0);

	glBindVertexArray(m_plane.m_VAO);
	glDrawElements(GL_TRIANGLES, m_plane.m_indexCount, GL_UNSIGNED_INT, 0);

	Gizmos::clear();
	Gizmos::addTransform(mat4(1), 10);

	vec4 white(1);
	vec4 black(0, 0, 0, 1);

	for (int i = 0; i <= 20; ++i){
		Gizmos::addLine(vec3(-10 + i, 0, -10), i != 10 ? vec3(-10 + i, 0, 10) : vec3(-10 + i, 0, 0), i != 10 ? black : white);
		Gizmos::addLine(vec3(-10, 0, -10 + i), i != 10 ? vec3(10, 0, -10 + i) : vec3(0, 0, -10 + i), i != 10 ? black : white);
	}
	Gizmos::draw(m_oCamera.getProjectionView());
	Application::draw();
}

void Shadows::BuildMeshes(){
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
	glGenBuffers(1, &m_bunny.m_VBO);
	glGenBuffers(1, &m_bunny.m_IBO);
	glBindVertexArray(m_bunny.m_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, m_bunny.m_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* vertexData.size(), vertexData.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bunny.m_IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)* mesh->indices.size(), mesh->indices.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(float)* mesh->positions.size()));

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	VertexNormal vertex_Data[4];
	vertex_Data[0].position = vec4(-10, 0, -10, 1);
	vertex_Data[1].position = vec4(-10, 0, 10, 1);
	vertex_Data[2].position = vec4(10, 0, 10, 1);
	vertex_Data[3].position = vec4(10, 0, -10, 1);

	vertex_Data[0].normal = vec4(0, 1, 0, 0);
	vertex_Data[1].normal = vec4(0, 1, 0, 0);
	vertex_Data[2].normal = vec4(0, 1, 0, 0);
	vertex_Data[3].normal = vec4(0, 1, 0, 0);

	vertex_Data[0].tangent = vec4(1, 0, 0, 0);
	vertex_Data[1].tangent = vec4(1, 0, 0, 0);
	vertex_Data[2].tangent = vec4(1, 0, 0, 0);
	vertex_Data[3].tangent = vec4(1, 0, 0, 0);


	vertex_Data[0].tex_coord = vec2(0, 0);
	vertex_Data[1].tex_coord = vec2(0, 1);
	vertex_Data[2].tex_coord = vec2(1, 1);
	vertex_Data[3].tex_coord = vec2(1, 0);

	unsigned int indexData[] = { 0, 1, 2, 0, 2, 3 };
	m_plane.m_indexCount = 6;

	glGenVertexArrays(1, &m_plane.m_VAO);
	glGenBuffers(1, &m_plane.m_VBO);
	glGenBuffers(1, &m_plane.m_IBO);
	glBindVertexArray(m_plane.m_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, m_plane.m_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(VertexNormal) * 4, vertex_Data, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_plane.m_IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 6, indexData, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(VertexNormal), 0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_TRUE, sizeof(VertexNormal), (void*)(sizeof(vec4)));
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_TRUE, sizeof(VertexNormal), (void*)(sizeof(vec4) * 2));
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(VertexNormal), (void*)(sizeof(vec4) * 3));

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Shadows::BuildShadowMap(){
	glGenFramebuffers(1, &m_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

	glGenTextures(1, &m_fboDepth);
	glBindTexture(GL_TEXTURE_2D, m_fboDepth);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_fboDepth, 0);

	glDrawBuffer(GL_NONE);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
		printf("Framebuffer Error!\n");

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Shadows::ReloadShader(){
	glDeleteProgram(m_diffuseShadowProgramID);
	glDeleteProgram(m_shadowProgramID);
	LoadShader("./data/shaders/diffuse_shadowed_vertex.glsl", 0, "./data/shaders/diffuse_shadowed_fragment.glsl", &m_diffuseShadowProgramID);
	LoadShader("./data/shaders/shadowed_vertex.glsl", 0, "./data/shaders/shadowed_fragment.glsl", &m_shadowProgramID);
}