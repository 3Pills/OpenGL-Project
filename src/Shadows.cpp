#include "Shadows.h"
#include "AntTweakBar.h"
#include "tiny_obj_loader.h"
#include "Utility.h"

Shadows::Shadows(): m_oCamera(50), m_lightExtents(50), m_lightNearZ(-20), m_lightFarZ(100){
	Application::Application();
}
Shadows::~Shadows(){}

bool Shadows::startup(){
	if (!Application::startup()){
		return false;
	}

	//AntTweakBar Initialisation.
	TwInit(TW_OPENGL_CORE, nullptr);
	TwWindowSize(m_iWidth, m_iHeight);

	//Binding input callbacks for ATB GUI.
	glfwSetMouseButtonCallback(m_window, OnMouseButton);
	glfwSetCursorPosCallback(m_window, OnMousePosition);
	glfwSetScrollCallback(m_window, OnMouseScroll);
	glfwSetKeyCallback(m_window, OnKey);
	glfwSetCharCallback(m_window, OnChar);
	glfwSetWindowSizeCallback(m_window, OnWindowResize);


	TwBar* m_lightingBar = TwNewBar("Lighting"); //Lighting window. Allows modification of lighting data.
	TwAddVarRW(m_lightingBar, "Direction", TW_TYPE_DIR3F, &m_lightDir, "");
	TwAddVarRW(m_lightingBar, "Extents", TW_TYPE_FLOAT, &m_lightExtents, "step=0.01 min=0");
	TwAddVarRW(m_lightingBar, "NearZ", TW_TYPE_FLOAT, &m_lightNearZ, "step=0.01");
	TwAddVarRW(m_lightingBar, "FarZ", TW_TYPE_FLOAT, &m_lightFarZ, "step=0.01 min=0");

	m_oCamera.SetPerspective(glm::radians(50.0f), 1280.0f / 720.0f, 0.1f, 20000.0f);

	Gizmos::create();

	LoadShader("./data/shaders/diffuse_shadowed.vs", 0, "./data/shaders/diffuse_shadowed.fs", &m_diffuseShadowProgramID);
	LoadShader("./data/shaders/shadowed.vs", 0, "./data/shaders/shadowed.fs", &m_shadowProgramID);

	BuildMeshes();
	BuildShadowMap();

	m_lightDir = glm::normalize(-glm::vec3(1, 2.5f, 1));

	return true;
}

bool Shadows::shutdown(){
	glDeleteVertexArrays(1, &m_bunny.m_VAO);
	glDeleteBuffers(1, &m_bunny.m_VBO);
	glDeleteVertexArrays(1, &m_plane.m_VAO);
	glDeleteBuffers(1, &m_plane.m_VBO);
	TwTerminate();
	Gizmos::destroy();
	return Application::shutdown();
}
bool Shadows::update(){
	if (!Application::update()){
		return false;
	}
	m_oCamera.Update(m_fDeltaTime);
	if (glfwGetKey(m_window, GLFW_KEY_R) == GLFW_PRESS){
		ReloadShader();
	}

	mat4 lightProj = glm::ortho<float>(-m_lightExtents, m_lightExtents, -m_lightExtents, m_lightExtents, m_lightNearZ, m_lightFarZ);
	mat4 lightView = glm::lookAt(vec3(0), m_lightDir, vec3(0, 1, 0));
	m_lightMatrix = lightProj * lightView * mat4(1);

	return true;
}
void Shadows::draw(){
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	
	glViewport(0, 0, 1024, 1024);
	glClear(GL_DEPTH_BUFFER_BIT);

	glEnable(GL_CULL_FACE);
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
	glUniformMatrix4fv(projView_uniform, 1, GL_FALSE, (float*)&m_oCamera.GetProjectionView());

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
	int shadowTexture_uniform = glGetUniformLocation(m_diffuseShadowProgramID, "shadowTexture");
	glUniform1i(shadowTexture_uniform, 1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_fboDepth);
	glActiveTexture(GL_TEXTURE1);
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
	Gizmos::draw(m_oCamera.GetProjectionView());
	TwDraw();
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
	vertexData.insert(vertexData.end(), mesh->normals.begin(), mesh->normals.end());
	vertexData.insert(vertexData.end(), mesh->texcoords.begin(), mesh->texcoords.end());
	

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
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(float)* mesh->positions.size()));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(float)* (mesh->positions.size() + mesh->normals.size())));
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(float)* (mesh->positions.size() + mesh->normals.size() + mesh->normals.size())));

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
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);

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
	LoadShader("./data/shaders/diffuse_shadowed.vs", 0, "./data/shaders/diffuse_shadowed.fs", &m_diffuseShadowProgramID);
	LoadShader("./data/shaders/shadowed.vs", 0, "./data/shaders/shadowed.fs", &m_shadowProgramID);
}