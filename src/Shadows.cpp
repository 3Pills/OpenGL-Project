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

	LoadShader("./shaders/diffuse_shadowed_vertex.glsl", 0, "./shaders/diffuse_shadowed_fragment.glsl", &m_diffuseShadowProgramID);
	LoadShader("./shaders/shadowed_vertex.glsl", 0, "./shaders/shadowed_fragment.glsl", &m_diffuseShadowProgramID);

	BuildMeshes();
	BuildShadowMap();
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

	return true;
}
void Shadows::draw(){
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	
	glViewport(0, 0, 1024, 1024);
	glClear(GL_DEPTH_BUFFER_BIT);

	glUseProgram(m_shadowProgramID);

	int lightMatrix_uniform = glGetUniformLocation(m_shadowProgramID, "lightMatrix");

	vec3 lightDir = glm::normalize(vec3(-1, -2.5f, -1));

	mat4 lightProj = glm::ortho(-10, 10, -10, 10, -10, 100);
	mat4 lightView = glm::lookAt(-lightDir, vec3(0), vec3(0,1,0));
	mat4 lightMatrix = lightProj * lightView;

	if (lightMatrix_uniform > -1)
		glUniformMatrix4fv(lightMatrix_uniform, 1, GL_FALSE, (float*)&lightMatrix);

	glBindVertexArray(m_bunny.m_VAO);
	glDrawElements(GL_TRIANGLES, m_bunny.m_indexCount, GL_UNSIGNED_INT, 0);

	glBindVertexArray(m_plane.m_VAO);
	glDrawElements(GL_TRIANGLES, m_plane.m_indexCount, GL_UNSIGNED_INT, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glViewport(0, 0, m_iWidth, m_iHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	Gizmos::clear();
	Gizmos::addTransform(mat4(1), 10);

	vec4 white(1);
	vec4 black(0, 0, 0, 1);

	for (int i = 0; i <= 20; ++i){
		Gizmos::addLine(vec3(-10 + i, 0, -10), i != 10 ? vec3(-10 + i, 0, 10) : vec3(-10 + i, 0, 0), i != 10 ? black : white);
		Gizmos::addLine(vec3(-10, 0, -10 + i), i != 10 ? vec3(10, 0, -10 + i) : vec3(0, 0, -10 + i), i != 10 ? black : white);
	}

	glUseProgram(m_diffuseShadowProgramID);

	mat4 offsetScale = mat4(
		0.5f, 0,	0,	  0,
		0,	  0.5f, 0,	  0,
		0,	  0,	0.5f, 0,
		0.5f, 0.5f, 0.5f, 1);

	lightMatrix = offsetScale * lightMatrix;

	int viewProj_uniform = glGetUniformLocation(m_diffuseShadowProgramID, "ViewProj");
	if (viewProj_uniform > -1)
		glUniformMatrix4fv(viewProj_uniform, 1, GL_FALSE, (float*)&m_oCamera.getProjectionView());

	int lightDir_uniform = glGetUniformLocation(m_diffuseShadowProgramID, "lightDir");
	if (lightDir_uniform > -1)
		glUniform3fv(lightDir_uniform, 1, (float*)&lightDir);

	glBindVertexArray(m_bunny.m_VAO);
	glDrawElements(GL_TRIANGLES, m_bunny.m_indexCount, GL_UNSIGNED_INT, 0);
	glBindVertexArray(m_plane.m_VAO);
	glDrawElements(GL_TRIANGLES, m_plane.m_indexCount, GL_UNSIGNED_INT, 0);

	Gizmos::draw(m_oCamera.getProjectionView());
	Application::draw();
}

void Shadows::BuildMeshes(){
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials; 

	tinyobj::LoadObj(shapes, materials, "./models/stanford/bunny.obj");

	m_bunny.m_indexCount = shapes[0].mesh.indices.size();

	tinyobj::mesh_t* mesh = &shapes[0].mesh;

	std::vector<float> vertexData;
	vertexData.reserve(mesh->positions.size() + mesh->normals.size());

	vertexData.insert(vertexData.begin(), mesh->positions.begin(), mesh->positions.end());
	vertexData.insert(vertexData.begin(), mesh->normals.begin(), mesh->normals.end());

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

	glGenVertexArrays(1, &m_plane.m_VAO);
	glGenBuffers(1, &m_plane.m_VBO);
	glGenBuffers(1, &m_plane.m_IBO);
	glBindVertexArray(m_plane.m_VAO);

	float planeVertexData[] = {
		-1, 0, -1, 1, 0, 1, 0,
		1, 0, -1, 1, 0, 1, 0,
		1, 0, 1, 1, 0, 1, 0,
		-1, 0, 1, 1, 0, 1, 0,
	};

	unsigned int planeIndexData[] = { 0, 1, 2, 0, 2, 3 };

	m_plane.m_indexCount = 6;

	glBindBuffer(GL_ARRAY_BUFFER, m_plane.m_VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_plane.m_IBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertexData), planeVertexData, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(planeIndexData), planeIndexData, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(float)* 6, 0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(float)* 6, (void*)(sizeof(float)* 4));

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
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}