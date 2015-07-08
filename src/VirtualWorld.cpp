#include "VirtualWorld.h"


VirtualWorld::VirtualWorld(): m_oCamera(50), m_LastKey(0){
	Application::Application();
}
VirtualWorld::~VirtualWorld(){}

bool VirtualWorld::startup(){
	if (!Application::startup()){
		return false;
	}

	m_oCamera.setPerspective(glm::radians(50.0f), 1280.0f / 720.0f, 0.1f, 20000.0f);
	m_aParticleEmitters.push_back(new GPUEmitter);

	for (GPUEmitter* particle : m_aParticleEmitters) {
		particle->Init(vec3(0), vec3(1), 100, 1.0f, 2.0f, 1.0f, 2.0f, 1.0f, 0.5f, 1.0f, 0.5f, vec4(1, 0.5, 0.5, 1), vec4(1, 0, 0, 0), EMIT_POINT, PMOVE_WAVE, "./data/textures/particles/glow.png");
	}
	//LoadShader("./data/shaders/particles_vertex.glsl", "", "./data/shaders/particles_fragment.glsl", &m_programID);

	Gizmos::create();
	return true;
}
bool VirtualWorld::shutdown(){
	for (GPUEmitter* particle : m_aParticleEmitters) {
		delete particle;
	}
	Gizmos::destroy();
	return Application::shutdown();
}
bool VirtualWorld::update(){
	if (!Application::update()){
		return false;
	}

	if (glfwGetKey(m_window, GLFW_KEY_R) == GLFW_PRESS && m_LastKey != GLFW_PRESS){
		ReloadShaders();
	}
	m_LastKey = glfwGetKey(m_window, GLFW_KEY_R);

	m_oCamera.update(m_fDeltaTime);
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
void VirtualWorld::draw(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//glUseProgram(m_programID);
	//int view_proj_uniform = glGetUniformLocation(m_programID, "ProjView");
	//if (view_proj_uniform > -1) {
	//	glUniformMatrix4fv(view_proj_uniform, 1, GL_FALSE, (float*)&m_oCamera.getProjectionView());
	//}

	//Opaque Drawing
	Gizmos::draw(m_oCamera.getProjectionView());
	
	//Transparency Drawing
	for (GPUEmitter* particle : m_aParticleEmitters) {
		particle->Render(m_fCurrTime, m_oCamera.getWorldTransform(), m_oCamera.getProjectionView());
	}

	//GUI Drawing
	Application::draw();
}



void VirtualWorld::ReloadShaders(){
	printf("Reloading Shaders...\n");
	//glDeleteProgram(m_programID);
	//LoadShader("./data/shaders/normal_mapped_vertex.glsl", "", "./data/shaders/normal_mapped_fragment.glsl", &m_programID);

	for (GPUEmitter* particle : m_aParticleEmitters) {
		particle->Reload();
	}
}