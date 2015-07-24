#ifndef _PHYS_X_H_
#define _PHYS_X_H_
#include "Application.h"
#include "Camera.h"

#include <PxPhysicsAPI.h>
#include <PxScene.h>

class PhysX : public Application
{
	FlyCamera m_oCamera;

	physx::PxFoundation* g_PhysicsFoundation;
	physx::PxPhysics* g_Physics;
	physx::PxScene* g_PhysicsScene;
	physx::PxDefaultErrorCallback gDefaultErrorCallback;
	physx::PxDefaultAllocator gDefaultAllocatorCallback;
	physx::PxSimulationFilterShader gDefaultFilterShader = physx::PxDefaultSimulationFilterShader;
	physx::PxMaterial* g_PhysicsMaterial;
	physx::PxMaterial* g_boxMaterial;
	physx::PxCooking* g_PhysicsCooker;

	physx::PxRigidDynamic* dynamicActors[32];
	physx::PxArticulation* g_PhysXActorsRagDolls[1];
public:
	PhysX();
	virtual ~PhysX();

	virtual bool startup();
	virtual bool shutdown();

	virtual bool update();
	virtual void draw();

	void setupPhysX();
	//void setupTutorial1();
	void setupVisualDebugger();

	void addWidget(physx::PxShape* shape, physx::PxRigidActor* actor);
	void addBox(physx::PxShape* pShape, physx::PxRigidActor* actor);

	void addCapsule(physx::PxShape* pShape, physx::PxRigidActor* actor);
};


#endif//_PHYS_X_H