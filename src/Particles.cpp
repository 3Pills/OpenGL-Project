#include "Particles.h"
#include "Utility.h"

Particles::Particles(): m_oCamera(50){
	Application::Application();
}
Particles::~Particles(){}

bool Particles::startup(){
	if (!Application::startup()){
		return false;
	}

	m_oCamera.SetPerspective(glm::radians(50.0f), 1280.0f / 720.0f, 0.1f, 20000.0f);
	m_emitter.Init(100000, vec3(-10, -10, -10), vec3(10, 10, 10), EMIT_OUTER_RING, 1000, 1, 2, 2, 4, 0.5f, 0.2f, vec4(1, 1, 0.65f, 1), vec4(1, 0.25f, 0, 1));

	LoadShader("./data/shaders/particles.vs", "", "./data/shaders/particles.fs", &m_programID);

	Gizmos::create();
	return true;
}
bool Particles::shutdown(){
	return Application::shutdown();
}
bool Particles::update(){
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
	m_emitter.Update(m_fDeltaTime, m_oCamera.GetWorldTransform());

	return true;
}
void Particles::draw(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(m_programID);
	int view_proj_uniform = glGetUniformLocation(m_programID, "ProjView");
	if (view_proj_uniform > -1) {
		glUniformMatrix4fv(view_proj_uniform, 1, GL_FALSE, (float*)&m_oCamera.GetProjectionView());
	}

	m_emitter.Render();

	Gizmos::draw(m_oCamera.GetProjectionView());
	Application::draw();
}