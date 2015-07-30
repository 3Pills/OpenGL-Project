#include "PhysModel.h"
#include <Gizmos.h>
#include "PhysScene.h"
#include "Utility.h"

PhysModel::PhysModel(const char* a_szModelPath, vec3 a_pos, PxGeometry* a_geometry, PxMaterial* a_physicsMaterial, 
					 PhysScene* a_physics, float a_density, float a_roughness, float a_fresnelScale, float a_physModelScale, mat4 a_modTransform, vec3 a_scale, quat a_rot) 
					 : FBXModel(a_szModelPath, a_roughness, a_fresnelScale, a_modTransform, a_pos, a_scale, a_rot), m_density(a_density), m_physModelScale(a_physModelScale), m_physMaterial(a_physicsMaterial) {
	if (a_geometry != nullptr) {
		PxTransform transform(PxVec3(a_pos.x, a_pos.y, a_pos.z));
		m_geometry = new PxGeometryHolder();
		m_geometry->storeAny(*a_geometry); //Store geometry data.
		m_phys = PxCreateDynamic(*a_physics->m_physics, transform, m_geometry->any(), *a_physicsMaterial, m_density);

		int numberShapes = m_phys->getNbShapes();
		PxShape* shapes;

		m_phys->getShapes(&shapes, numberShapes);

		vec3 modelExtents = vec3(0);
		switch (m_geometry->getType()) {
		case PxGeometryType::eSPHERE:
			modelExtents = vec3(0, m_geometry->sphere().radius, 0);
			break;
		case PxGeometryType::eCAPSULE:
			//Gizmos::addCylinderFilled(m_pos, m_geometry->sphere().radius, 6, 6, vec4(0, 1, 0, 1));
			modelExtents = vec3(-m_geometry->capsule().halfHeight - m_geometry->sphere().radius, 0, 0);
			m_modTransform = (glm::rotate(glm::radians(90.0f), vec3(0, 0, 1))) * m_modTransform;
			break;
		case PxGeometryType::eBOX:
			modelExtents = vec3(0, m_geometry->box().halfExtents.y, 0);
		}
		PxTransform relativePose = PxTransform((*(PxVec3*)(&modelExtents[0])));
		shapes->setLocalPose(relativePose);

		a_physics->m_physicsScene->addActor(*m_phys);
		//m_modTransform = (glm::translate(-modelExtents)) * m_modTransform;
	}
	else {
		//AttachRigidBodyConvex(a_density, a_physics, a_physicsMaterial, a_pos);
	}
}

PhysModel::~PhysModel() {
	FBXModel::~FBXModel();
	delete m_geometry;
}

void PhysModel::RenderGizmos() {
	FBXModel::RenderGizmos();
}

void PhysModel::Update(float dt) {
	PxTransform transform = m_phys->getGlobalPose();
	m_pos = (*(vec3*)(&transform.p[0]));
	m_rot = quat(transform.q.w, transform.q.x, transform.q.y, transform.q.z);
	FBXModel::Update(dt);
}

/*
void PhysModel::RenderGizmos() {
	FBXModel::RenderGizmos();
	int numberShapes = m_phys->getNbShapes();
	PxShape** shapes = new PxShape*[numberShapes] {};
	int shapeCount = m_phys->getShapes(shapes, numberShapes);
	for (int i = 0; i < shapeCount; i++) {
		PxTransform transform = shapes[i]->getLocalPose();
		vec3 pos = (*(vec3*)(&transform.p[0]));
		quat rot = quat(transform.q.w, transform.q.x, transform.q.y, transform.q.z);

		mat4 local = glm::toMat4(m_rot) * glm::translate(pos);
		pos = local[3].xyz;

		switch (shapes[i]->getGeometryType()){
		case PxGeometryType::eSPHERE:
			Gizmos::addSphere(m_pos + pos, shapes[i]->getGeometry().sphere().radius, 12, 12, vec4(0), &(mat4(local)));
			break;
		case PxGeometryType::eCAPSULE:
			Gizmos::addCapsule(m_pos + pos, shapes[i]->getGeometry().capsule().halfHeight * 2, shapes[i]->getGeometry().capsule().radius, 12, 12, vec4(1), &mat4(local));
			break;
		case PxGeometryType::eBOX:
			PxVec3 extents = shapes[i]->getGeometry().box().halfExtents;
			Gizmos::addAABB(m_pos + pos, (*(vec3*)(&extents[0])), vec4(1), &mat4(m_rot));
			break;
		}
	}
	delete[] shapes;
	}*/