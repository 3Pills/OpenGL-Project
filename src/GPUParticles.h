#ifndef GPUPARTICLES_H_
#define GPUPARTICLES_H_
#include "Application.h"
#include "Camera.h"
#include "GPUEmitter.h"

class GPUParticles : public Application
{
	FlyCamera m_oCamera;

	GPUEmitter m_emitter;

	unsigned int m_programID;
public:
	GPUParticles();
	virtual ~GPUParticles();

	virtual bool startup();
	virtual bool shutdown();

	virtual bool update();
	virtual void draw();
};

#endif//GPUPARTICLES_H_