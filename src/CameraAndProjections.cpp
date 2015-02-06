#include "CameraAndProjections.h"
#include <iostream>

using namespace glm;

CameraAndProjections::CameraAndProjections(): m_oCamera(0.1){
	Application::Application();
}
CameraAndProjections::~CameraAndProjections(){}

bool CameraAndProjections::startup(){
	if (!Application::startup()){
		return false;
	}

	m_pos = vec3(5, 5, 0);
	m_look = vec3(0, 0, 0);
	//m_view = glm::lookAt(m_pos, m_look, vec3(0, 1, 0));
	//m_proj = glm::perspective(radians(90.0f), 1280.0f / 720.0f, 0.1f, 20000.0f);

	//m_oCamera.setLookAt(m_pos, m_look, vec3(0, 1, 0));
	m_oCamera.setPerspective(radians(50.0f), 1280.0f / 720.0f, 0.1f, 20000.0f);

	Gizmos::create();
	return true;
}
bool CameraAndProjections::shutdown(){
	return Application::shutdown();
}
bool CameraAndProjections::update(){
	if (!Application::update()){
		return false;
	}
	//m_pos.x = sinf(m_fCurrTime) * 10;
	//m_pos.z = cosf(m_fCurrTime) * 10;
	//m_view = glm::lookAt(m_pos, m_look, vec3(0, 1, 0));

	return true;
}
void CameraAndProjections::draw(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	Gizmos::clear();
	Gizmos::addTransform(mat4(1), 10);

	//glm::mat4 inverse = glm::inverse(m_view);
	//glm::translate(inverse, inverse[2].xyz * m_fDeltaTime);
	vec4 white(1);
	vec4 black(0, 0, 0, 1);

	for (int i = 0; i <= 20; ++i){
		Gizmos::addLine(vec3(-10 + i, 0, -10), i != 10 ? vec3(-10 + i, 0, 10) : vec3(-10 + i, 0, 0), i != 10 ? black : white);
		Gizmos::addLine(vec3(-10, 0, -10 + i), i != 10 ? vec3(10, 0, -10 + i) : vec3(0, 0, -10 + i), i != 10 ? black : white);
	}
	m_oCamera.update(m_fDeltaTime);

	Gizmos::draw(m_oCamera.getProjectionView());
	Application::draw();
}