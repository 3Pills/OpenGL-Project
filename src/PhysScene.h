#ifndef _PHYS_SCENE_H_
#define _PHYS_SCENE_H_

#include <PxPhysicsAPI.h>
#include <PxScene.h>
#include "glm_header.h"

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

	void Update(const float dt, const bool a_renderGizmos);

	PxRigidStatic*	AddRigidBodyStatic(		const PxTransform a_transform, PxGeometry* a_geometry, PxMaterial* a_physMaterial, void* a_userData = nullptr);
	PxRigidDynamic* AddRigidBodyDynamic(	const PxTransform a_transform, PxGeometry* a_geometry, PxMaterial* a_physMaterial, void* a_userData = nullptr, const float a_density = 100.f);
	PxRigidStatic*	AttachRigidBodyTriangle(const PxTransform a_transform, PxMaterial* a_physicsMaterial, void* a_userData, const float a_physModelScale = 1.f);
	PxRigidDynamic* AttachRigidBodyConvex(	const PxTransform a_transform, PxMaterial* a_physicsMaterial, void* a_userData, const float a_density = 100.f, const float a_physModelScale = 1.f);

	PxRigidStatic*	AddHeightMap(float* a_heightMap, PxMaterial* a_physMaterial, glm::vec2 a_dims, glm::vec3 a_scale);
	PxCloth*		AddCloth(const glm::vec3& a_pos, unsigned int& a_vertexCount, unsigned int& a_indexCount, const glm::vec3* a_vertices, unsigned int* a_indices);
};

#endif//_PHYS_SCENE