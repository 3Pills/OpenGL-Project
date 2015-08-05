#ifndef _PHYS_SCENE_H_
#define _PHYS_SCENE_H_

#include <PxPhysicsAPI.h>
#include <PxScene.h>
#include "glm_header.h"
#include "Camera.h"

using namespace physx;

class PhysScene {
	PxFoundation* m_physicsFoundation;
	PxDefaultErrorCallback m_defaultErrorCallback;
	PxDefaultAllocator m_defaultAllocatorCallback;
	PxSimulationFilterShader m_defaultFilterShader = PxDefaultSimulationFilterShader;
	PxVisualDebuggerConnection* m_debuggerConnection;

	PxRigidStatic* m_plane[6];

public:
	PxPhysics* m_physics;
	PxScene* m_physicsScene;
	PxCooking* m_cooking;

	PhysScene();
	~PhysScene();

	void Update(const float dt, const bool a_renderGizmos);
	void AddWorldBounds(const vec3 a_extents);

	PxRigidStatic*	AddRigidBodyStatic(		const PxTransform a_transform, PxGeometry* a_geometry, PxMaterial* a_physMaterial, void* a_userData = nullptr);
	PxRigidDynamic* AddRigidBodyDynamic(	const PxTransform a_transform, PxGeometry* a_geometry, PxMaterial* a_physMaterial, void* a_userData = nullptr, const float a_density = 100.f);
	PxRigidStatic*	AttachRigidBodyTriangle(const PxTransform a_transform, PxMaterial* a_physicsMaterial, void* a_userData, const float a_physModelScale = 1.f);
	PxRigidDynamic* AttachRigidBodyConvex(	const PxTransform a_transform, PxMaterial* a_physicsMaterial, void* a_userData, const float a_density = 100.f, const float a_physModelScale = 1.f);

	PxRigidStatic*	AddHeightMap(float* a_heightMap, PxMaterial* a_physMaterial, glm::vec2 a_dims, glm::vec3 a_scale, unsigned int a_downScale);
	PxCloth*		AddCloth(const glm::vec3& a_pos, unsigned int& a_vertexCount, unsigned int& a_indexCount, const glm::vec3* a_vertices, unsigned int* a_indices);

	PxController*	AddPlayerController(const PxExtendedVec3 a_pos, PxMaterial* a_physMaterial, void* a_userData);
};

//Overload of the PxUserControllerHitReport for use in controlling the player.
class PlayerHitReport : public PxUserControllerHitReport {
	PxVec3 m_playerContactNormal;
public:
	PlayerHitReport() :PxUserControllerHitReport(){};

	virtual void onShapeHit(const PxControllerShapeHit &hit);
	virtual void onControllerHit(const PxControllersHit &hit);
	virtual void onObstacleHit(const PxControllerObstacleHit &hit);

	PxVec3 getPlayerContactNormal(){ return m_playerContactNormal; };
	void clearPlayerContactNormal(){ m_playerContactNormal = PxVec3(0, 0, 0); };
};

struct ClothData {
	PxCloth* m_cloth;

	vec3* m_vertices, *m_normals;
	vec2* m_texCoords;
	unsigned int* m_indices;

	unsigned int m_program, m_texture;
	unsigned int m_indexCount, m_vertexCount;
	unsigned int m_VAO, m_VBO, m_textureVBO, m_IBO;
	unsigned int m_rows, m_columns;

	ClothData(const unsigned int a_particleSize, const unsigned int a_rows, const unsigned int a_columns);
	~ClothData();

	void GenerateGLBuffers();
	void Render(Camera* a_camera);
};

#endif//_PHYS_SCENE