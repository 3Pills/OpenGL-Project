#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#include "gl_core_4_4.h"
#include <GLFW\glfw3.h>
#include "Gizmos.h"
#include "glm_header.h"

class Application {
protected:
	GLFWwindow* m_window;

	float m_fCurrTime, m_fDeltaTime, m_fPrevTime;

	vec3 m_pos;
	vec3 m_look;
	mat4 m_view;
	mat4 m_proj;
public:
	Application();
	virtual ~Application();

	virtual bool startup();
	virtual bool shutdown();

	virtual bool update();
	virtual void draw();
};
#endif//_APPLICATION_H_