#ifndef _APP_BASE_H_
#define _APP_BASE_H_
#include "Application.h"
#include "Camera.h"
#include "Vertex.h"

#include "AntTweakBar.h"

class AdvancedTextures : public Application {
	FlyCamera m_oCamera;
	OpenGLInfo m_glData;

	unsigned int m_programID, m_LastKey;
	unsigned int m_diffTex, m_normTex, m_specTex;
	vec3 m_vAmbCol, m_vLightCol, m_vLightPos;
	vec4 m_vBgCol;
	float m_fSpecPow, m_fFPS;
public:
	AdvancedTextures();
	virtual ~AdvancedTextures();

	virtual bool startup();
	virtual bool shutdown();

	virtual bool update();
	virtual void draw();
	void LoadTextures();
	void GenerateQuad(const float a_fSize);
	void ReloadShader();
};

#endif//_APP_BASE_H_