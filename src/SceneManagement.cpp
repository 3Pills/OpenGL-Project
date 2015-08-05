#include "SceneManagement.h"
#include "tiny_obj_loader.h"
#include "Utility.h"
#include "BoundingVolumes.h"

SceneManagement::SceneManagement(): m_oCamera(50){
	Application::Application();
}
SceneManagement::~SceneManagement(){}

bool SceneManagement::startup(){
	if (!Application::startup()){
		return false;
	}

	m_oCamera.SetPerspective(glm::radians(50.0f), 1280.0f / 720.0f, 0.1f, 20000.0f);

	LoadMesh("./data/models/stanford/bunny.obj");
	float extents = 1000.0f;

	for (unsigned int i = 0; i < 10000; ++i) {
		m_meshes.push_back(m_meshes[0]);
		m_meshes.back().m_transform =
			glm::translate(vec3(glm::linearRand(-extents, extents), glm::linearRand(-extents, extents), glm::linearRand(-extents, extents))) *
			glm::rotate(glm::linearRand(-6.f, 6.f), glm::normalize(vec3(glm::linearRand(-10, 10), glm::linearRand(-10, 10), glm::linearRand(-10, 10))));
	}

	LoadShader("./data/shaders/lighting_vertex.glsl", 0, "./data/shaders/lighting_fragment.glsl", &m_programID);

	m_ambCol = vec3(0.1f);
	m_lightDir = vec3(0, -1, 0);
	m_lightCol = vec3(0.6f, 0, 0);
	m_matCol = vec3(1);
	m_specPow = 15;

	Gizmos::create();
	return true;
}

bool SceneManagement::shutdown(){
	Gizmos::destroy();
	return Application::shutdown();
}

bool SceneManagement::update(){
	if (!Application::update()){
		return false;
	}
	m_oCamera.Update(m_fDeltaTime);

	return true;
}

void SceneManagement::draw(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	Gizmos::clear();
	Gizmos::addTransform(mat4(1), 10);

	vec4 white(1);
	vec4 black(0, 0, 0, 1);

	for (int i = 0; i <= 20; ++i){
		Gizmos::addLine(vec3(-10 + i, 0, -10), i != 10 ? vec3(-10 + i, 0, 10) : vec3(-10 + i, 0, 0), i != 10 ? black : white);
		Gizmos::addLine(vec3(-10, 0, -10 + i), i != 10 ? vec3(10, 0, -10 + i) : vec3(0, 0, -10 + i), i != 10 ? black : white);
	}

	glUseProgram(m_programID);

	int viewProj_uniform = glGetUniformLocation(m_programID, "projView");
	if (viewProj_uniform > -1) {
		glUniformMatrix4fv(viewProj_uniform, 1, GL_FALSE, (float*)&m_oCamera.GetProjectionView());
	}
	int ambCol_uniform = glGetUniformLocation(m_programID, "ambCol");
	if (ambCol_uniform > -1) {
		glUniform3fv(ambCol_uniform, 1, (float*)&m_ambCol);
	}
	int matCol_uniform = glGetUniformLocation(m_programID, "matCol");
	if (matCol_uniform > -1) {
		glUniform3fv(matCol_uniform, 1, (float*)&m_matCol);
	}
	int difCol_uniform = glGetUniformLocation(m_programID, "lightCol");
	if (difCol_uniform > -1) {
		glUniform3fv(difCol_uniform, 1, (float*)&m_lightCol);
	}
	int lightDir_uniform = glGetUniformLocation(m_programID, "lightDir");
	if (lightDir_uniform > -1) {
		glUniform3fv(lightDir_uniform, 1, (float*)&m_lightDir);
	}
	int camPos_uniform = glGetUniformLocation(m_programID, "camPos");
	if (camPos_uniform > -1) {
		glUniform3fv(camPos_uniform, 1, (float*)&m_oCamera.GetWorldTransform()[3].xyz);
	}
	int specPow_uniform = glGetUniformLocation(m_programID, "specPow");
	if (specPow_uniform > -1) {
		glUniform1f(specPow_uniform, m_specPow);
	}
	int world_uniform = glGetUniformLocation(m_programID, "world");

	vec4 planes[6];
	m_oCamera.GetFrustumPlanes(planes);

	for (unsigned int mesh_index = 0; mesh_index < m_meshes.size(); ++mesh_index) {
		bool isInFrustum = true;

		for (unsigned int plane_index = 0; plane_index < 6; ++plane_index) {
			if (!OnPlanePositive(planes[plane_index], m_meshes[mesh_index].m_aabb, m_meshes[mesh_index].m_transform)) {
				isInFrustum = false;
				break;
			}
		}

		if (isInFrustum) {
			glUniformMatrix4fv(world_uniform, 1, GL_FALSE, (float*)&m_meshes[mesh_index].m_transform);
			DrawMesh(m_meshes[mesh_index]);
		}
		RenderPlane(m_meshes[mesh_index].m_transform[3]);
	}

	Gizmos::draw(m_oCamera.GetProjectionView());
	Application::draw();
}

void RenderAABB(AABB a_aabb){
	vec3 center = (a_aabb.m_vMin + a_aabb.m_vMax) * 0.5f;
	vec3 extents = a_aabb.m_vMin - a_aabb.m_vMax;

	Gizmos::addAABB(center, extents, vec4(1));
}

void SceneManagement::LoadMesh(const char* obj_filename){
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	tinyobj::LoadObj(shapes, materials, obj_filename);
	unsigned int meshCount = m_meshes.size();

	m_meshes.resize(m_meshes.size() + shapes.size());
	
	for (unsigned int shapeIndex = 0; shapeIndex < shapes.size(); ++shapeIndex){
		unsigned int meshIndex = meshCount + shapeIndex;
		std::vector<float> vertexData;

		unsigned int floatCount = shapes[shapeIndex].mesh.positions.size();
		floatCount += shapes[shapeIndex].mesh.normals.size();

		vertexData.reserve(floatCount);
		vertexData.insert(vertexData.end(), shapes[shapeIndex].mesh.positions.begin(), shapes[shapeIndex].mesh.positions.end());
		vertexData.insert(vertexData.end(), shapes[shapeIndex].mesh.normals.begin(), shapes[shapeIndex].mesh.normals.end());

		m_meshes[meshIndex].m_data.m_indexCount = shapes[shapeIndex].mesh.indices.size();

		glGenVertexArrays(1, &m_meshes[meshIndex].m_data.m_VAO);
		glGenBuffers(1, &m_meshes[meshIndex].m_data.m_VBO);
		glGenBuffers(1, &m_meshes[meshIndex].m_data.m_IBO);

		glBindVertexArray(m_meshes[meshIndex].m_data.m_VAO);
		glBindBuffer(GL_ARRAY_BUFFER, m_meshes[meshIndex].m_data.m_VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float)* floatCount, vertexData.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_meshes[meshIndex].m_data.m_IBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, shapes[shapeIndex].mesh.indices.size() * sizeof(unsigned int), shapes[shapeIndex].mesh.indices.data(), GL_STATIC_DRAW);

		glEnableVertexAttribArray(0); //position
		glEnableVertexAttribArray(1); //normal data

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, 0, (void*)(sizeof(float)* shapes[shapeIndex].mesh.positions.size()));

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		m_meshes[meshIndex].m_aabb = GenerateAABB((vec3*)&shapes[shapeIndex].mesh.positions[0], shapes[shapeIndex].mesh.positions.size() / 3);
	}
}

void SceneManagement::DrawMesh(MeshObject mesh)
{
	glBindVertexArray(mesh.m_data.m_VAO);
	glDrawElements(GL_TRIANGLES, mesh.m_data.m_indexCount, GL_UNSIGNED_INT, 0);
}