#include "Animation.h"
#include "Utility.h"

Animation::Animation() : m_oCamera(50), m_vAmbCol(vec3(0.4f)), m_vLightCol(vec3(0.85f)), m_vLightPos(vec3(0, -10, 0)), m_fSpecPow(16){
	Application::Application();
}
Animation::~Animation(){}

bool Animation::startup(){
	if (!Application::startup()){
		return false;
	}
	glEnable(GL_DEPTH_TEST);

	TwInit(TW_OPENGL_CORE, nullptr);
	TwWindowSize(1280, 720);

	glfwSetMouseButtonCallback(m_window, OnMouseButton);
	glfwSetCursorPosCallback(m_window, OnMousePosition);
	glfwSetScrollCallback(m_window, OnMouseScroll);

	glfwSetKeyCallback(m_window, OnKey);
	glfwSetCharCallback(m_window, OnChar);

	glfwSetWindowSizeCallback(m_window, OnWindowResize);

	m_oCamera.setPerspective(glm::radians(50.0f), 1280.0f / 720.0f, 0.1f, 20000.0f);

	m_model = new FBXModel("./data/models/characters/Pyro/pyro.fbx");

	TwBar* m_bar = TwNewBar("Settings");
	TwAddVarRW(m_bar, "Ambient", TW_TYPE_COLOR3F, &m_vAmbCol, "group=Light");
	TwAddVarRW(m_bar, "Direction", TW_TYPE_DIR3F, &m_vLightPos, "group=Light");
	TwAddVarRW(m_bar, "Color", TW_TYPE_COLOR3F, &m_vLightCol, "group=Light");
	TwAddVarRW(m_bar, "Specular Power", TW_TYPE_FLOAT, &m_fSpecPow, "group=Light min=0.05 step=0.05 max=1000.0");

	Gizmos::create();
	return true;
}

bool Animation::shutdown(){
	delete m_model;
	TwTerminate();
	return Application::shutdown();
}

bool Animation::update(){
	if (!Application::update()){
		return false;
	}
	m_oCamera.update(m_fDeltaTime);

	m_model->m_ambCol = m_vAmbCol;
	m_model->m_lightDir = -m_vLightPos;
	m_model->m_lightCol = m_vLightCol;
	m_model->m_specPow = m_fSpecPow;
	m_model->Update(m_fCurrTime);

	if (glfwGetKey(m_window, GLFW_KEY_R) == GLFW_PRESS){
		ReloadShader();
	}

	vec4 white(1);
	vec4 black(0, 0, 0, 1);

	Gizmos::clear();
	Gizmos::addTransform(mat4(1), 10);
	for (int i = 0; i <= 20; ++i){
		Gizmos::addLine(vec3(-10 + i, 0, -10), i != 10 ? vec3(-10 + i, 0, 10) : vec3(-10 + i, 0, 0), i != 10 ? black : white);
		Gizmos::addLine(vec3(-10, 0, -10 + i), i != 10 ? vec3(10, 0, -10 + i) : vec3(0, 0, -10 + i), i != 10 ? black : white);
	}

	return true;
}
void Animation::draw(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_model->Render(m_oCamera);

	//for (unsigned int i = 0; i < skeleton->m_boneCount; ++i) {
	//	skeleton->m_nodes[i]->updateGlobalTransform();
	//	mat4 node_global = skeleton->m_nodes[i]->m_globalTransform;
	//	vec3 node_pos = node_global[3].xyz;
	//	Gizmos::addAABBFilled(node_pos, vec3(4.f), vec4(1, 0, 0, 1), &node_global);
	//
	//	if (skeleton->m_nodes[i]->m_parent != nullptr){
	//		vec3 parent_pos = skeleton->m_nodes[i]->m_parent->m_globalTransform[3].xyz;
	//		Gizmos::addLine(node_pos, parent_pos, vec4(0, 1, 0, 1));
	//	}
	//}

	Gizmos::draw(m_oCamera.getProjectionView());
	TwDraw();
	Application::draw();
}

void Animation::ReloadShader(){
	m_model->ReloadShader();
}