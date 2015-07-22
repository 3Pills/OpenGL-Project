#ifndef PBR_H_
#define PBR_H_
#include "Application.h"
#include "Camera.h"

#include "AntTweakBar.h"
#include "FBXModel.h"

class PBR : public Application
{
	FlyCamera m_oCamera;

	FBXModel* m_model;

	vec3 m_ambCol, m_lightCol, m_lightPos;
	float m_roughness, m_fresnelScale;
public:
	PBR();
	virtual ~PBR();

	virtual bool startup();
	virtual bool shutdown();

	virtual bool update();
	virtual void draw();

	void ReloadShader();
};

#endif//PBR_H_