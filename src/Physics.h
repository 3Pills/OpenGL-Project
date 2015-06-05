#ifndef PHYSICS_H_
#define PHYSICS_H_
#include "Application.h"
#include "Camera.h"
#include "PhysicsObject.h"

class Physics : public Application {
	FlyCamera m_oCamera;
public:
	Physics();
	virtual ~Physics();

	virtual bool startup();
	virtual bool shutdown();

	virtual bool update();
	virtual void draw();
};

#endif//PHYSICS_H_