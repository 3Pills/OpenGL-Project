#ifndef _INTRO_TO_OPENGL_H_
#define _INTRO_TO_OPENGL_H_
#include "Application.h"

class IntroToOpenGL : public Application {
	vec3 m_pos;
	vec3 m_look;
	mat4 m_view;
	mat4 m_proj;
public:
	IntroToOpenGL();
	virtual ~IntroToOpenGL();

	virtual bool startup();
	virtual bool shutdown();

	virtual bool update();
	virtual void draw();
};

#endif//_INTRO_TO_OPENGL_H_