#include "GPUParticles.h"
#include "Utility.h"
#include "AntTweakBar.h"

GPUParticles::GPUParticles(): m_oCamera(50){
	Application::Application();
}
GPUParticles::~GPUParticles(){}

bool GPUParticles::startup(){
	if (!Application::startup()){
		return false;
	}

	TwInit(TW_OPENGL_CORE, nullptr);
	TwWindowSize(1280, 720);

	glfwSetMouseButtonCallback(m_window, OnMouseButton);
	glfwSetCursorPosCallback(m_window, OnMousePosition);
	glfwSetScrollCallback(m_window, OnMouseScroll);

	glfwSetKeyCallback(m_window, OnKey);
	glfwSetCharCallback(m_window, OnChar);

	glfwSetWindowSizeCallback(m_window, OnWindowResize);

	m_oCamera.setPerspective(glm::radians(50.0f), 1280.0f / 720.0f, 0.1f, 20000.0f);
	m_emitter.Init(vec3(0), 100000, 1, 10, 10, 100, 0.05f, 0.2f, vec4(1, 1, 0.65f, 1), vec4(1, 0.25f, 0, 1), EMIT_OUTER_RING);

	TwEnumVal emitTypes[] = { { EMIT_POINT, "Point" }, { EMIT_LINE, "Line" },
	{ EMIT_PLANE, "Plane" }, { EMIT_RING, "Ring" }, { EMIT_OUTER_RING, "Outer Ring" },
	{ EMIT_RECTANGLE, "Rectangle" }, { EMIT_OUTER_RECTANGLE, "Outer Rectangle" },
	{ EMIT_SPHERE, "Sphere" }, { EMIT_OUTER_SPHERE, "Outer Sphere" } };

	TwType emitType = TwDefineEnum("EmitType", emitTypes, 9);

	TwBar* m_bar = TwNewBar("Particle Settings");
	//TwAddVarRW(m_bar, "Emit Type", emitType, &m_emitter.m_emitType, "");
	TwAddVarRW(m_bar, "LMin", TW_TYPE_FLOAT, &m_emitter.m_lifespanMin, "step=0.05 min=0.0 group=Lifespan");
	TwAddVarRW(m_bar, "LMax", TW_TYPE_FLOAT, &m_emitter.m_lifespanMax, "step=0.05 min=0.0 group=Lifespan");
	TwAddVarRW(m_bar, "VMin", TW_TYPE_FLOAT, &m_emitter.m_velocityMin, "step=0.05 min=0.0 group=Velocity");
	TwAddVarRW(m_bar, "VMax", TW_TYPE_FLOAT, &m_emitter.m_velocityMax, "step=0.05 min=0.0 group=Velocity");
	TwAddVarRW(m_bar, "SStart", TW_TYPE_FLOAT, &m_emitter.m_startSize, "step=0.05 min=0.0 group=Size");
	TwAddVarRW(m_bar, "SEnd", TW_TYPE_FLOAT, &m_emitter.m_endSize, "step=0.05 min=0.0 group=Size");
	TwAddVarRW(m_bar, "CStart", TW_TYPE_COLOR4F, &m_emitter.m_startColor, "group=Color");
	TwAddVarRW(m_bar, "CEnd", TW_TYPE_COLOR4F, &m_emitter.m_endColor, "group=Color");

	Gizmos::create();
	return true;
}
bool GPUParticles::shutdown(){
	TwTerminate();
	Gizmos::destroy();
	return Application::shutdown();
}
bool GPUParticles::update(){
	if (!Application::update()){
		return false;
	}
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
void GPUParticles::draw(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_emitter.Render(m_fCurrTime, m_oCamera.getWorldTransform(), m_oCamera.getProjectionView());

	TwDraw();
	Gizmos::draw(m_oCamera.getProjectionView());
	Application::draw();
}