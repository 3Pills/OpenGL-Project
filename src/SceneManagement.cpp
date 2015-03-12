#include "SceneManagement.h"
#include <iostream>
#include "tiny_obj_loader.h"

SceneManagement::SceneManagement(): m_oCamera(50){
	Application::Application();
}
SceneManagement::~SceneManagement(){}

bool SceneManagement::startup(){
	if (!Application::startup()){
		return false;
	}

	m_oCamera.setPerspective(glm::radians(50.0f), 1280.0f / 720.0f, 0.1f, 20000.0f);

	for (unsigned int i = 0; i < m_meshes.size(); ++i){

	}

	LoadShader("./shaders/lighting_vertex.glsl", "", "./shaders/lighting_fragment.glsl", &m_programID);

	Gizmos::create();
	return true;
}

bool SceneManagement::shutdown(){
	return Application::shutdown();
}

bool SceneManagement::update(){
	if (!Application::update()){
		return false;
	}
	m_oCamera.update(m_fDeltaTime);

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

	Gizmos::draw(m_oCamera.getProjectionView());
	Application::draw();
}

void RenderAABB(AABB aabb){
	vec3 center = (aabb.m_vMin + aabb.m_vMax) * 0.5f;
	vec3 extents = aabb.m_vMin - aabb.m_vMax;

	Gizmos::addAABB(center, extents, vec4(1))
}

MeshObject SceneManagement::LoadMesh(const char* obj_filename){
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	tinyobj::LoadObj(shapes, materials, obj_filename);

	m_meshes.resize(m_meshes.size() + shapes.size());
	
	for (unsigned int i = 0; i < shapes.size(); ++i){
		std::vector<float> vertex_data;

		unsigned int floatCount = shapes[i].mesh.positions.size();
		floatCount += shapes[i].mesh.normals.size();

		vertex_data.reserve(floatCount);
		vertex_data.insert(vertex_data.end(), shapes[i].mesh.positions.begin(), shapes[i].mesh.positions.end());
		vertex_data.insert(vertex_data.end(), shapes[i].mesh.normals.begin(), shapes[i].mesh.normals.end());

		m_meshes[i].m_indexCount = shapes[i].mesh.indices.size();

		glGenVertexArrays(1, &m_meshes[i].m_VAO);
		glGenBuffers(1, &m_meshes[i].m_VBO);
		glGenBuffers(1, &m_meshes[i].m_IBO);

		glBindVertexArray(m_meshes[i].m_VAO);
		glBindBuffer(GL_ARRAY_BUFFER, m_meshes[i].m_VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float)* floatCount, vertex_data.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_meshes[i].m_IBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, shapes[i].mesh.indices.size() * sizeof(unsigned int), shapes[i].mesh.indices.data(), GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);//position
		glEnableVertexAttribArray(1);//normal data

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, 0, (void*)(sizeof(float)* shapes[i].mesh.positions.size()));

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		m_meshes[mesh_index].m_aabb = GenerateAABB((vec3*)&shapes[i].mesh.positions[0], shapes[i].mesh.positions.size() / 3);
	}
}