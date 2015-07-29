#ifndef _PHYS_MODEL_H_
#define _PHYS_MODEL_H_
#include "FBXModel.h"
#include <PxPhysicsAPI.h>
#include <PxScene.h>

using namespace physx;

class PhysScene;

class PhysModel : public FBXModel {
	PxMaterial* m_physMaterial;
	PxGeometryHolder* m_geometry;
	PxRigidDynamic* m_phys;
	float m_density, m_physModelScale;
public:
	PhysModel(const char* a_szModelPath, vec3 a_pos, PxGeometry* a_geometry, PxMaterial* a_physicsMaterial, PhysScene* a_physics,
			  float a_density = 10.0f, float a_roughness = 0.3f, float a_fresnelScale = 2.0f, float a_physModelScale = 1.0f, 
			  mat4 a_modTransform = mat4(1), vec3 a_scale = vec3(1), quat a_rot = quat());
	virtual ~PhysModel();
	virtual void Update(float dt);
	virtual void RenderGizmos();

	void AttachRigidBodyConvex(float a_density, PhysScene* a_physics, PxMaterial* a_physicsMaterial, vec3 a_pos);
};


#endif//_PHYS_MODEL_H_