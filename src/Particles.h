#ifndef PARTICLES_H_
#define PARTICLES_H_
#include "Application.h"
#include "Camera.h"
#include "Emitter.h"

class Particles : public Application
{
	FlyCamera m_oCamera;

	Emitter m_emitter;

	unsigned int m_programID;
public:
	Particles();
	virtual ~Particles();

	virtual bool startup();
	virtual bool shutdown();

	virtual bool update();
	virtual void draw();
};

#endif//PARTICLES_H_