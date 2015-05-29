#ifndef VIRTUAL_WORLD_H_
#define VIRTUAL_WORLD_H_
#include "Application.h"
#include "Camera.h"
#include <vector>
#include "GPUEmitter.h"

class VirtualWorld : public Application
{
	FlyCamera m_oCamera;
	std::vector<GPUEmitter*> m_aParticleEmitters;
	unsigned int m_programID;
public:
	VirtualWorld();
	virtual ~VirtualWorld();

	virtual bool startup();
	virtual bool shutdown();

	virtual bool update();
	virtual void draw();
};

#endif//VIRTUAL_WORLD_H_