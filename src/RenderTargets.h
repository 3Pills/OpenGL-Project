#ifndef RENDER_TARGETS_H_
#define RENDER_TARGETS_H_
#include "Application.h"
#include "Camera.h"
#include "Vertex.h"

class RenderTargets : public Application
{
	FlyCamera m_oCamera;
	OpenGLData m_plane;

	unsigned int m_fbo, m_fboTexture, m_fboDepth;
	unsigned int m_programID;
public:
	RenderTargets();
	virtual ~RenderTargets();

	virtual bool startup();
	virtual bool shutdown();

	virtual bool update();
	virtual void draw();

	void GenerateFramebuffer();
	void GeneratePlane();
};

#endif//RENDER_TARGETS_H_