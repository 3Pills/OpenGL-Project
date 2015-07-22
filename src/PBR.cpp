#include "PBR.h"
#include "Utility.h"

PBR::PBR() : m_oCamera(50), m_ambCol(vec3(0.4f)), m_lightCol(vec3(0.85f)), m_lightPos(vec3(0, -10, 0)), m_fresnelScale(1){
	Application::Application();
}
PBR::~PBR(){}

bool PBR::startup(){
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
	TwAddVarRW(m_bar, "Ambient", TW_TYPE_COLOR3F, &m_ambCol, "group=Light");
	TwAddVarRW(m_bar, "Direction", TW_TYPE_DIR3F, &m_lightPos, "group=Light");
	TwAddVarRW(m_bar, "Color", TW_TYPE_COLOR3F, &m_lightCol, "group=Light");
	TwAddVarRW(m_bar, "Roughness", TW_TYPE_FLOAT, &m_roughness, "group=Light min=0 step=0.05 max=100.0");
	TwAddVarRW(m_bar, "Fresnel Scale", TW_TYPE_FLOAT, &m_fresnelScale, "group=Light min=0 step=0.05 max=100.0");

	Gizmos::create();
	return true;
}

bool PBR::shutdown(){
	delete m_model;
	TwTerminate();
	return Application::shutdown();
}

bool PBR::update(){
	if (!Application::update()){
		return false;
	}
	m_oCamera.update(m_fDeltaTime);

	m_model->m_ambCol = m_ambCol;
	m_model->m_lightDir = -m_lightPos;
	m_model->m_lightCol = m_lightCol;
	m_model->m_roughness = m_roughness;
	m_model->m_fresnelScale = m_fresnelScale;
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
void PBR::draw(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_model->Render(m_oCamera);

	Gizmos::draw(m_oCamera.getProjectionView());
	TwDraw();
	Application::draw();
}

void PBR::ReloadShader(){
	m_model->ReloadShader();
}