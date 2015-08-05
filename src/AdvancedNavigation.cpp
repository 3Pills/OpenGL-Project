#include "AdvancedNavigation.h"
#include "Utility.h"

AdvancedNavigation::AdvancedNavigation(): m_oCamera(50){
	Application::Application();
}
AdvancedNavigation::~AdvancedNavigation(){}

bool AdvancedNavigation::startup(){
	if (!Application::startup()){
		return false;
	}

	m_oCamera.SetPerspective(glm::radians(50.0f), 1280.0f / 720.0f, 0.1f, 20000.0f);
	m_sponzaMesh = LoadOBJ("./data/data/models/sponza/SponzaSimple.obj");

	//LoadShader();

	Gizmos::create();
	return true;
}
bool AdvancedNavigation::shutdown(){
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

	return true;
}
void AdvancedNavigation::draw(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Gizmos::draw(m_oCamera.GetProjectionView());
	Application::draw();
}