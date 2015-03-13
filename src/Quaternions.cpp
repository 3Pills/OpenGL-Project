#include "Quaternions.h"

Quaternions::Quaternions() : m_oCamera(50) {
	Application::Application();
}

Quaternions::~Quaternions() {}

bool Quaternions::startup() {
	if (!Application::startup()) 
		return false;

	m_oCamera.setPerspective(glm::radians(50.0f), 1280.0f / 720.0f, 0.1f, 20000.0f);

	Gizmos::create();
	glm::quat boringQuaternion(1, 0, 0, 0);
	glm::quat eulerQuat(vec3(3, 5, 7));

	float PI = 3.14159f;

	m_hipFrames[0].position = vec3(0, 5, 0);
	m_hipFrames[0].rotation = quat(vec3(-1, 0, 0));

	m_kneeFrames[0].position = vec3(0, -2.5, 0);
	m_kneeFrames[0].rotation = quat(vec3(-1, 0, 0));

	m_ankleFrames[0].position = vec3(0, -2.5, 0);
	m_ankleFrames[0].rotation = quat(vec3(0, 0, 0));

	m_hipFrames[1].position = vec3(0, 5, 0);
	m_hipFrames[1].rotation = quat(vec3(1, 0, 0));

	m_kneeFrames[1].position = vec3(0, -2.5, 0);
	m_kneeFrames[1].rotation = quat(vec3(0, 0, 0));

	m_ankleFrames[1].position = vec3(0, -2.5, 0);
	m_ankleFrames[1].rotation = quat(vec3(0, 0, 0));

	m_hipFrames[2].position = vec3(0, 5, 0);
	m_hipFrames[2].rotation = quat(vec3(0, 0, 0));

	m_kneeFrames[2].position = vec3(0, -2.5, 0);
	m_kneeFrames[2].rotation = quat(vec3(0, 0, 0));

	m_ankleFrames[2].position = vec3(0, -2.5, 0);
	m_ankleFrames[2].rotation = quat(vec3(0, 0, 0));

	return true;
}

mat4 EvaluateKeyframes(KeyFrame start, KeyFrame end, float t){
	vec3 pos = glm::mix(start.position, end.position, t);
	quat rot = glm::slerp(start.rotation, end.rotation, t);
	return (glm::translate(pos) * glm::toMat4(rot));
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

	
	if ((int)m_fCurrTime % 2 == 0) {
		m_hipBone = EvaluateKeyframes(m_hipFrames[0], m_hipFrames[1], m_fCurrTime);
		m_kneeBone = EvaluateKeyframes(m_kneeFrames[0], m_kneeFrames[1], m_fCurrTime);
		m_ankleBone = EvaluateKeyframes(m_ankleFrames[0], m_ankleFrames[1], m_fCurrTime);
	}
	else{
		m_hipBone = EvaluateKeyframes(m_hipFrames[0], m_hipFrames[2], m_fCurrTime);
		m_kneeBone = EvaluateKeyframes(m_kneeFrames[0], m_kneeFrames[2], m_fCurrTime);
		m_ankleBone = EvaluateKeyframes(m_ankleFrames[0], m_ankleFrames[2], m_fCurrTime);
	}

	mat4 global_knee = m_hipBone * m_kneeBone;
	mat4 global_ankle = global_knee * m_ankleBone;

	vec3 hip_pos = m_hipBone[3].xyz;
	vec3 knee_pos = global_knee[3].xyz;
	vec3 ankle_pos = global_ankle[3].xyz;

	Gizmos::addSphere(hip_pos, 0.35f, 8, 8, vec4(0, 0, 1, 1), &m_hipBone);
	Gizmos::addSphere(knee_pos, 0.35f, 8, 8, vec4(1, 0, 0, 1), &global_knee);
	Gizmos::addSphere(ankle_pos, 0.35f, 8, 8, vec4(0, 1, 0, 1), &global_ankle);

	Gizmos::addLine(hip_pos, knee_pos, vec4(1, 1, 1, 1));
	Gizmos::addLine(ankle_pos, knee_pos, vec4(1, 1, 1, 1));

	for (int i = 0; i <= 20; ++i) {
		Gizmos::addLine(vec3(-10 + i, 0, -10), i != 10 ? vec3(-10 + i, 0, 10) : vec3(-10 + i, 0, 0), i != 10 ? black : white);
		Gizmos::addLine(vec3(-10, 0, -10 + i), i != 10 ? vec3(10, 0, -10 + i) : vec3(0, 0, -10 + i), i != 10 ? black : white);
	}

	Gizmos::draw(m_oCamera.getProjectionView());
	Application::draw();
}