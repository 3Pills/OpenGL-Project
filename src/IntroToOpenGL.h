#ifndef _INTRO_TO_OPENGL_H_
#define _INTRO_TO_OPENGL_H_
#include "Application.h"
class IntroToOpenGL : public Application {
public:
	IntroToOpenGL();
	virtual ~IntroToOpenGL();

	virtual bool startup();
	virtual bool shutdown();

	virtual bool update();
	virtual void draw();
};
#endif//_INTRO_TO_OPENGL_H_