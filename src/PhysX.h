#ifndef _PHYS_X_H_
#define _PHYS_X_H_
#include "Application.h"
#include "Camera.h"

#include <PxPhysicsAPI.h>
#include <PxScene.h>
#include <vector>

using namespace physx;

class PhysX : public Application
{
	FlyCamera m_oCamera;

	PxFoundation* m_physicsFoundation;
	PxPhysics* m_physics;
	PxScene* m_physicsScene;
	PxDefaultErrorCallback m_defaultErrorCallback;
	PxDefaultAllocator m_defaultAllocatorCallback;
	PxSimulationFilterShader m_defaultFilterShader = PxDefaultSimulationFilterShader;
	PxMaterial* m_physicsMaterial;
	PxMaterial* m_boxMaterial;
	PxCooking* m_physicsCooker;
	PxD6Joint* m_testD6Joint;

	float currentXSpeed;
	float currentYSpeed;

	std::vector<PxRigidActor*> m_dynamicActors;
	std::vector<vec3> m_extents;
	PxArticulation* m_physXActorsRagDolls[1];
public:
	PhysX();
	virtual ~PhysX();

	virtual bool startup();
	virtual bool shutdown();

	virtual bool update();
	virtual void draw();

	void setupPhysX();
	void setupMotor();
	void setupVisualDebugger();

	void addWidget(physx::PxShape* shape, physx::PxRigidActor* actor);
	void addBox(physx::PxShape* pShape, physx::PxRigidActor* actor);

	void addCapsule(physx::PxShape* pShape, physx::PxRigidActor* actor);
};


#endif//_PHYS_X_H