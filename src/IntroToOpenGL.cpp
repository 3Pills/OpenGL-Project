#include "IntroToOpenGL.h"
#include "Gizmos.h"
#include <iostream>

using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat4;

IntroToOpenGL::IntroToOpenGL(){}
IntroToOpenGL::~IntroToOpenGL(){}

bool IntroToOpenGL::startup(){
	if (Application::startup() == false) {
		return false;
	}
	m_pos = vec3(1000, 5, 0);
	m_look = vec3(0, 5, 0);
	m_view = glm::lookAt(m_pos, m_look, vec3(0, 1, 0));
	m_proj = glm::perspective(glm::radians(90.0f), 1280.0f / 720.0f, 0.1f, 20000.0f);

	Gizmos::create();
}

bool IntroToOpenGL::shutdown(){
	Gizmos::destroy();
	return Application::shutdown();
}

bool IntroToOpenGL::update(){
	if (!Application::update()) {
		return false;
	}
	m_view = glm::lookAt(m_pos, m_look, vec3(0, 1, 0));
	m_pos.x = sinf(m_fCurrTime) * 10;
	m_pos.z = cosf(m_fCurrTime) * 10;

		//Gizmos::addAABBFilled(vec3(0, 4, -5), vec3(1,1,1), blue);

		//Gizmos::addAABBFilled(vec3(0, 0.5, 0.5), vec3(0.5, 0.5, 0.5), greytwo);
		//Gizmos::addAABBFilled(vec3(0, 1, 2.5), vec3(0.5, 1, 1.5), greytwo);
		//Gizmos::addAABBFilled(vec3(0, 3, 3.5), vec3(0.5, 1, 1.5), red);
		//Gizmos::addAABBFilled(vec3(0, 3, 8.5), vec3(0.5, 1, 1.5), red);
		//Gizmos::addAABBFilled(vec3(0, 1, 9.5), vec3(0.5, 1, 1.5), greytwo);
		//Gizmos::addAABBFilled(vec3(0, 0.5, 11.5), vec3(0.5, 0.5, 0.5), greytwo);
	return true;
}
void IntroToOpenGL::draw(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	Gizmos::clear();
	Gizmos::addTransform(mat4(1), 10);

	vec4 red(0.5, 0, 0, 1);
	vec4 yellow(1, 1, 0, 1);
	vec4 grey(0.5, 0.5, 0.2, 1);
	vec4 greytwo(0.25, 0.25, 0.1, 1);
	vec4 green(0, 1, 0, 1);
	vec4 blue(0, 0, 1, 1);
	vec4 white(1);
	vec4 black(0, 0, 0, 1);

	for (int i = 0; i <= 20; ++i){
		Gizmos::addLine(vec3(-10 + i, 0, -10), i != 10 ? vec3(-10 + i, 0, 10) : vec3(-10 + i, 0, 0), i != 10 ? black : white);
		Gizmos::addLine(vec3(-10, 0, -10 + i), i != 10 ? vec3(10, 0, -10 + i) : vec3(0, 0, -10 + i), i != 10 ? black : white);
	}
	mat4 sun_transform = glm::rotate(m_fCurrTime, vec3(0, 1, 0));
	mat4 child_one_transform = sun_transform * glm::translate(vec3(4.5, 0, 0)) * glm::rotate(m_fCurrTime * 2.f, vec3(0, 1, 0));
	mat4 child_two_transform = child_one_transform * glm::translate(vec3(0, 0, 0));

	Gizmos::addSphere(sun_transform[3].xyz, 4, 16, 16, blue, &sun_transform);
	Gizmos::addSphere(child_two_transform[3].xyz, 1, 16, 16, black, &child_two_transform);

	Gizmos::addTri(vec3(0, 9, 0), vec3(0, 10, -2), vec3(0, 10, 0), green);

	Gizmos::draw(m_proj, m_view);
	Application::draw();
}