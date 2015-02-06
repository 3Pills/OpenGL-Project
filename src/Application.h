#ifndef _APPLICATION_H_
#define _APPLICATION_H_
#include "gl_core_4_4.h"
#include <GLFW\glfw3.h>
#include "Gizmos.h"
#define GLM_SWIZZLE
#include "glm/glm.hpp"
#include "glm/ext.hpp"
class Application {
protected:
	GLFWwindow* m_window;
	float m_fCurrTime, m_fDeltaTime, m_fPrevTime;
	glm::vec3 m_pos;
	glm::vec3 m_look;
	glm::mat4 m_view;
	glm::mat4 m_proj;

public:
	Application();
	virtual ~Application();

	virtual bool startup();
	virtual bool shutdown();

	virtual bool update();
	virtual void draw();
};
#endif//_APPLICATION_H_