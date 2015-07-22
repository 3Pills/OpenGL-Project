#ifndef ANIMATION_H_
#define ANIMATION_H_
#include "Application.h"
#include "Camera.h"

#include "AntTweakBar.h"
#include "FBXModel.h"

class Animation : public Application
{
	FlyCamera m_oCamera;

	FBXModel* m_model;

	vec3 m_vAmbCol, m_vLightCol, m_vLightPos;
	float m_fSpecPow;
public:
	Animation();
	virtual ~Animation();

	virtual bool startup();
	virtual bool shutdown();

	virtual bool update();
	virtual void draw();

	void ReloadShader();
};

#endif//ANIMATION_H_