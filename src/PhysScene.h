#ifndef _PHYS_SCENE_H_
#define _PHYS_SCENE_H_

#include <PxPhysicsAPI.h>
#include <PxScene.h>

using namespace physx;

class PhysScene {
	PxFoundation* m_physicsFoundation;
	PxDefaultErrorCallback m_defaultErrorCallback;
	PxDefaultAllocator m_defaultAllocatorCallback;
	PxSimulationFilterShader m_defaultFilterShader = PxDefaultSimulationFilterShader;
	PxVisualDebuggerConnection* m_debuggerConnection;

	PxTransform m_planePose;
	PxRigidStatic* m_plane;

public:
	PxPhysics* m_physics;
	PxScene* m_physicsScene;

	PhysScene();
	~PhysScene();

	void Update(float dt);
};

#endif//_PHYS_SCENE