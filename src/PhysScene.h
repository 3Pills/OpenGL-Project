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
	PxCooking* m_cooking;

	PhysScene();
	~PhysScene();

	void Update(float dt, bool a_renderGizmos);

	void AddRigidBodyStatic(PxTransform a_transform, PxGeometry* a_geometry, PxMaterial* a_physMaterial, void* a_userData = nullptr, float a_density = 100.f);
	void AddRigidBodyDynamic(PxTransform a_transform, PxGeometry* a_geometry, PxMaterial* a_physMaterial, void* a_userData = nullptr, float a_density = 100.f);
	void AttachRigidBodyConvex(PxTransform a_transform, PxMaterial* a_physicsMaterial, void* a_userData, float a_density = 100.f, float a_physModelScale = 1.f);
};

#endif//_PHYS_SCENE