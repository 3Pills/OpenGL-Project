#include "Quaternions.h"
#include <iostream>

Quaternions::Quaternions() : m_oCamera(50) {
	Application::Application();
}

Quaternions::~Quaternions() {}

bool Quaternions::startup() {
	if (!Application::startup()) 
		return false;

	m_pos = vec3(5, 5, 0);
	m_look = vec3(0, 0, 0);

	m_oCamera.setPerspective(glm::radians(50.0f), 1280.0f / 720.0f, 0.1f, 20000.0f);

	Gizmos::create();
	glm::quat boringQuaternion(1, 0, 0, 0);
	glm::quat eulerQuat(vec3(3, 5, 7));

	m_vPositions[0] = vec3(10, 5, 10);
	m_vPositions[1] = vec3(-10, 0, -10);
	
	m_qRotations[0] = quat(vec3(-1, -1, -1));
	m_qRotations[1] = quat(vec3(1, 1, 1));

	return true;
}

bool Quaternions::shutdown() {
	return Application::shutdown();
}

bool Quaternions::update() {
	if (!Application::update())
		return false;

	m_oCamera.update(m_fDeltaTime);

	return true;
}

void Quaternions::draw() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	Gizmos::clear();
	Gizmos::addTransform(mat4(1), 10);

	vec4 white(1);
	vec4 black(0, 0, 0, 1);

	float fSinWave = sinf(m_fCurrTime) * 0.5f + 0.5f;
	vec3 vFinalPos = glm::mix(m_vPositions[0], m_vPositions[1], fSinWave);
	quat qFinatRot = glm::slerp(m_qRotations[0], m_qRotations[1], fSinWave);
	mat4 transform = glm::translate(vFinalPos) * glm::toMat4(qFinatRot);

	Gizmos::addTransform(transform);
	Gizmos::addAABBFilled(vFinalPos, vec3(1), vec4(0, 0, 1, 1), &transform);

	for (int i = 0; i <= 20; ++i) {
		Gizmos::addLine(vec3(-10 + i, 0, -10), i != 10 ? vec3(-10 + i, 0, 10) : vec3(-10 + i, 0, 0), i != 10 ? black : white);
		Gizmos::addLine(vec3(-10, 0, -10 + i), i != 10 ? vec3(10, 0, -10 + i) : vec3(0, 0, -10 + i), i != 10 ? black : white);
	}

	Gizmos::draw(m_oCamera.getProjectionView());
	Application::draw();
}