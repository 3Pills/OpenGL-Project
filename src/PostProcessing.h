#ifndef POST_PROCESSING_H_
#define POST_PROCESSING_H_
#include "Application.h"
#include "Camera.h"
#include "Vertex.h"

class PostProcessing : public Application
{
	FlyCamera m_oCamera;

	unsigned int m_fbo, m_fboTexture, m_fboDepth;
	unsigned int m_postProgramID;
	OpenGLData m_quad;
public:
	PostProcessing();
	virtual ~PostProcessing();

	virtual bool startup();
	virtual bool shutdown();

	virtual bool update();
	virtual void draw();

	void GenerateFramebuffer();
	void GenerateQuad();
};

#endif//POST_PROCESSING_H_