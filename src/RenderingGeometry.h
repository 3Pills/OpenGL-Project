#ifndef _RENDER_GEO_H_
#define _RENDER_GEO_H_
#include "Application.h"
#include "Camera.h"

class RenderingGeometry : public Application {
	FlyCamera m_oCamera;
public:
	RenderingGeometry();
	virtual ~RenderingGeometry();

	virtual bool startup();
	virtual bool shutdown();

	virtual bool update();
	virtual void draw();

	void generateGrid(const unsigned int rows, const unsigned int cols);

	unsigned int m_programID;
	unsigned int m_indexCount;
	unsigned int m_VAO;
	unsigned int m_VBO;
	unsigned int m_IBO;
};

#endif//_RENDER_GEO_H_