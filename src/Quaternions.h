#ifndef QUATERNIONS_H_
#define QUATERNIONS_H_
#include "Application.h"
#include "Camera.h"

struct KeyFrame {
	glm::vec3 position;
	glm::quat rotation;
};

class Quaternions : public Application
{
	FlyCamera m_oCamera;

	vec3 m_vPositions[2];
	quat m_qRotations[2];

	KeyFrame m_hipFrames[3];
	KeyFrame m_kneeFrames[3];
	KeyFrame m_ankleFrames[3];

	mat4 m_hipBone;
	mat4 m_kneeBone;
	mat4 m_ankleBone;
public:
	Quaternions();
	virtual ~Quaternions();

	virtual bool startup();
	virtual bool shutdown();

	virtual bool update();
	virtual void draw();
};

#endif//QUATERNIONS_H_