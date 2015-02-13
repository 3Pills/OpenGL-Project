#ifndef _LIGHTING_H_
#define _LIGHTING_H_
#include "Application.h"
#include "Camera.h"
#include "tiny_obj_loader.h"
#include <string>
#include <vector>
#include "Vertex.h"

class Lighting : public Application {
	FlyCamera m_oCamera;
	unsigned int m_programID;
	unsigned int m_LastKey;

	vec3 m_vAmbCol, m_vLightCol, m_vLightPos, m_vMatCol;
	float m_fSpecPow;

	std::vector<OpenGLInfo> m_glData;
public:
	Lighting();
	virtual ~Lighting();

	virtual bool startup();
	virtual bool shutdown();

	virtual bool update();
	virtual void draw();

	void CreateOpenGLBuffers(std::vector<tinyobj::shape_t>& shapes);
	void CleanOpenGLBuffers();

	void ReloadShader();
};

#endif//_LIGHTING_H_