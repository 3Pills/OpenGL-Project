#include "AdvancedNavigation.h"
#include "Utility.h"

AdvancedNavigation::AdvancedNavigation(): m_oCamera(50){
	Application::Application();
}
AdvancedNavigation::~AdvancedNavigation(){}

void AdvancedNavigation::processNodes(NavMeshNode a_first, NavMeshNode a_second) {
	if (&a_first == &a_second) return;

	//Wrap vertice arrays in a shorter variable name.
	vec3* A = a_first.vertice;
	vec3* B = a_second.vertice;

	//Check for corresponding edge between both meshes
	//A-B comparison to XYZ vertices
	if ((A[0] == B[0] && A[1] == B[1]) || (A[0] == B[1] && A[1] == B[2]) || (A[0] == B[2] && A[1] == B[0]) ||
		(A[0] == B[1] && A[1] == B[0]) || (A[0] == B[2] && A[1] == B[1]) || (A[0] == B[0] && A[1] == B[2])) {
		a_first.edgeTargets[0] = &a_second;
	}
	//B-C
	if ((A[1] == B[0] && A[2] == B[1]) || (A[1] == B[1] && A[2] == B[2]) || (A[1] == B[2] && A[2] == B[0]) ||
		(A[1] == B[1] && A[2] == B[0]) || (A[1] == B[2] && A[2] == B[1]) || (A[1] == B[0] && A[2] == B[2])) {
		a_first.edgeTargets[0] = &a_second;
	}
	//A-C
	if ((A[0] == B[0] && A[2] == B[1]) || (A[0] == B[1] && A[2] == B[2]) || (A[0] == B[2] && A[2] == B[0]) ||
		(A[0] == B[1] && A[2] == B[0]) || (A[0] == B[2] && A[2] == B[1]) || (A[0] == B[0] && A[2] == B[2])) {
		a_first.edgeTargets[0] = &a_second;
	}
}

bool AdvancedNavigation::startup(){
	if (!Application::startup()){
		return false;
	}

	m_oCamera.SetPerspective(glm::radians(50.0f), 1280.0f / 720.0f, 0.1f, 20000.0f);
	m_sponzaMesh = new FBXModel("./data/models/sponza/SponzaSimple.fbx");
	m_sponzaMesh->m_lightDir = vec3(-1);
	m_sponzaMesh->m_lightCol = vec3(1);

	m_sponzaNavMesh = new FBXModel("./data/models/sponza/SponzaSimpleNavMesh.fbx");
	m_sponzaNavMesh->m_lightDir = vec3(0,-1,0);
	m_sponzaNavMesh->m_lightCol = vec3(1);

	for (unsigned int i = 0; i < m_sponzaNavMesh->m_file->getMeshCount(); i++) {
		FBXMeshNode* meshNode = m_sponzaNavMesh->m_file->getMeshByIndex(i);

		for (unsigned int i = 0; i < meshNode->m_children.size(); i++) {
			FBXNode* node = meshNode->m_children[i];
		}

		m_navMesh.push_back(NavMeshNode());
		m_navMesh.back().vertice[0] = meshNode->m_vertices[0].position.xyz;
		m_navMesh.back().vertice[1] = meshNode->m_vertices[1].position.xyz;
		m_navMesh.back().vertice[2] = meshNode->m_vertices[2].position.xyz;
		m_navMesh.back().position   = meshNode->m_globalTransform[3].xyz;
	}

	for (NavMeshNode first : m_navMesh) {
		for (NavMeshNode second : m_navMesh) {
			processNodes(first, second);
		}
	}

	//LoadShader();

	Gizmos::create();
	return true;
}
bool AdvancedNavigation::shutdown(){
	delete m_sponzaMesh;
	Gizmos::destroy();
	return Application::shutdown();
}
bool AdvancedNavigation::update(){
	if (!Application::update()){
		return false;
	}
	m_oCamera.Update(m_fDeltaTime);
	Gizmos::clear();
	Gizmos::addTransform(mat4(1), 10);

	vec4 white(1);
	vec4 black(0, 0, 0, 1);

	for (int i = 0; i <= 20; ++i){
		Gizmos::addLine(vec3(-10 + i, 0, -10), i != 10 ? vec3(-10 + i, 0, 10) : vec3(-10 + i, 0, 0), i != 10 ? black : white);
		Gizmos::addLine(vec3(-10, 0, -10 + i), i != 10 ? vec3(10, 0, -10 + i) : vec3(0, 0, -10 + i), i != 10 ? black : white);
	}
	
	for (NavMeshNode node : m_navMesh) {
		Gizmos::addAABBFilled(node.position, vec3(0.5f), vec4(1, 0, 0, 1));
		Gizmos::addLine(node.position, node.edgeTargets[0]->position, vec4(1, 0, 0, 1));
	}

	return true;
}
void AdvancedNavigation::draw(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	m_sponzaMesh->Render(&m_oCamera);
	m_sponzaNavMesh->Render(&m_oCamera);
	Gizmos::draw(m_oCamera.GetProjectionView());
	Application::draw();
}