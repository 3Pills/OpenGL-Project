#include "PhysModel.h"
#include <Gizmos.h>
#include "PhysScene.h"
#include "Utility.h"

PhysModel::PhysModel(const char* a_szModelPath, vec3 a_pos, PxGeometry* a_geometry, PxMaterial* a_physicsMaterial, 
					 PhysScene* a_physics, float a_density, float a_roughness, float a_fresnelScale, mat4 a_modTransform, vec3 a_scale, quat a_rot) 
		: FBXModel(a_szModelPath, a_pos, a_roughness, a_fresnelScale, a_modTransform, a_scale, a_rot), m_density(a_density) {
	PxTransform transform(PxVec3(a_pos.x, a_pos.y, a_pos.z));
	m_geometry = new PxGeometryHolder();
	m_geometry->storeAny(*a_geometry); //Store geometry data.
	m_phys = PxCreateDynamic(*a_physics->m_physics, transform, m_geometry->any(), *a_physicsMaterial, m_density);
	a_physics->m_physicsScene->addActor(*m_phys);

	vec3 modelExtents = vec3(0);
	switch (m_geometry->getType()) {
		case PxGeometryType::eSPHERE:
			modelExtents = vec3(0, m_geometry->sphere().radius, 0);
			break;
		case PxGeometryType::eCAPSULE:
			//Gizmos::addCylinderFilled(m_pos, m_geometry->sphere().radius, 6, 6, vec4(0, 1, 0, 1));
			break;
		case PxGeometryType::eBOX:
			modelExtents = vec3(0, m_geometry->box().halfExtents.y, 0);
	}
	m_modTransform = (glm::translate(-modelExtents)) * m_modTransform;
}

PhysModel::~PhysModel() {
	FBXModel::~FBXModel();
	delete m_geometry;
}

void PhysModel::RenderGizmos() {
	FBXModel::RenderGizmos();
	switch (m_geometry->getType()){
	case PxGeometryType::eSPHERE:
		Gizmos::addSphere(m_pos, m_geometry->sphere().radius, 12, 12, vec4(0), &mat4(m_rot));
		break;
	case PxGeometryType::eCAPSULE:
		//Gizmos::addCylinderFilled(m_pos, m_geometry->sphere().radius, 6, 6, vec4(0, 1, 0, 1));
		break;
	case PxGeometryType::eBOX:
		PxVec3 extents = m_geometry->box().halfExtents;
		Gizmos::addAABB(m_pos, vec3(extents.x, extents.y, extents.z), vec4(1), &mat4(m_rot));
		break;
	}
}

void PhysModel::Update(float dt) {
	PxTransform transform = m_phys->getGlobalPose();
	m_pos = vec3(transform.p.x, transform.p.y, transform.p.z);
	m_rot = quat(transform.q.w, transform.q.x, transform.q.y, transform.q.z);
	FBXModel::Update(dt);
}