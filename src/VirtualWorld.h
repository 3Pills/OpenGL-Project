#ifndef VIRTUAL_WORLD_H_
#define VIRTUAL_WORLD_H_
#include "Application.h"
#include "Camera.h"
#include <vector>
#include "GPUEmitter.h"
#include "FBXModel.h"

class VirtualWorld : public Application
{
	FlyCamera m_oCamera;
	std::vector<GPUEmitter*> m_aParticleEmitters;
	std::vector<FBXModel*> m_aFBXModels;

	unsigned int m_programID, m_LastKey;
public:
	VirtualWorld();
	virtual ~VirtualWorld();

	virtual bool startup();
	virtual bool shutdown();

	virtual bool update();
	virtual void draw();

	void ReloadShaders();
};

#endif//VIRTUAL_WORLD_H_