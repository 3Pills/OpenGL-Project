#ifndef QUATERNIONS_H_
#define QUATERNIONS_H_
#include "Application.h"
#include "Camera.h"

class Quaternions : public Application
{
	FlyCamera m_oCamera;

	vec3 m_vPositions[2];
	quat m_qRotations[2];
public:
	Quaternions();
	virtual ~Quaternions();

	virtual bool startup();
	virtual bool shutdown();

	virtual bool update();
	virtual void draw();
};

#endif//QUATERNIONS_H_