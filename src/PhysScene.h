#ifndef _PHYS_SCENE_H_
#define _PHYS_SCENE_H_

#include <PxPhysicsAPI.h>
#include <PxScene.h>
#include "glm_header.h"
#include "Camera.h"
#include "Utility.h"

using namespace physx;

class PhysScene {
	static PxFoundation* m_physicsFoundation;
	static PxDefaultErrorCallback m_defaultErrorCallback;
	static PxDefaultAllocator m_defaultAllocatorCallback;
	static PxSimulationFilterShader m_defaultFilterShader;
	static PxVisualDebuggerConnection* m_debuggerConnection;

	static PxRigidStatic* m_plane[6];

public:
	static PxPhysics* m_physics;
	static PxScene* m_physicsScene;
	static PxCooking* m_cooking;

	static void Init();
	static void Shutdown();

	static void Update(const float dt, const bool a_renderGizmos);
	static void AddWorldBounds(const vec3 a_extents);

	static PxRigidStatic*	AddRigidBodyStatic(		const PxTransform a_transform, PxGeometry* a_geometry, PxMaterial* a_physMaterial, void* a_userData = nullptr);
	static PxRigidDynamic*	AddRigidBodyDynamic(	const PxTransform a_transform, PxGeometry* a_geometry, PxMaterial* a_physMaterial, void* a_userData = nullptr, const float a_density = 100.f);
	static PxRigidStatic*	AttachRigidBodyTriangle(const PxTransform a_transform, PxMaterial* a_physicsMaterial, void* a_userData, const float a_physModelScale = 1.f);
	static PxRigidDynamic*	AttachRigidBodyConvex(	const PxTransform a_transform, PxMaterial* a_physicsMaterial, void* a_userData, const float a_density = 100.f, const float a_physModelScale = 1.f);

	static PxCloth*			AddCloth(const glm::vec3& a_pos, unsigned int& a_vertexCount, unsigned int& a_indexCount, const glm::vec3* a_vertices, unsigned int* a_indices);
	static PxRigidStatic*	AddHeightMap(float* a_heightMap, PxMaterial* a_physMaterial, glm::vec2 a_dims, glm::vec3 a_scale, unsigned int a_downScale);

	static PxController*	AddPlayerController(const PxExtendedVec3 a_pos, PxMaterial* a_physMaterial, void* a_userData);
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

	vec3 m_nailPos;
	vec3* m_vertices, *m_normals;
	vec2* m_texCoords;
	unsigned int* m_indices;

	unsigned int m_program, m_texture;
	unsigned int m_indexCount, m_vertexCount;
	unsigned int m_rows, m_columns;
	OpenGLData m_buffers; //OpenGL buffers

	ClothData(const unsigned int a_particleSize, const unsigned int a_rows, const unsigned int a_columns, const char* a_filename);
	~ClothData();

	void GenerateGLBuffers();
	void Render(Camera* a_camera);
};

#endif//_PHYS_SCENE