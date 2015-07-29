#include "PhysModel.h"
#include <Gizmos.h>
#include "PhysScene.h"
#include "Utility.h"

PhysModel::PhysModel(const char* a_szModelPath, vec3 a_pos, PxGeometry* a_geometry, PxMaterial* a_physicsMaterial, 
					 PhysScene* a_physics, float a_density, float a_roughness, float a_fresnelScale, float a_physModelScale, mat4 a_modTransform, vec3 a_scale, quat a_rot) 
		: FBXModel(a_szModelPath, a_pos, a_roughness, a_fresnelScale, a_modTransform, a_scale, a_rot), m_density(a_density), m_physModelScale(a_physModelScale), m_physMaterial(a_physicsMaterial) {
	if (a_geometry != nullptr) {
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
	else {
		AttachRigidBodyConvex(a_density, a_physics, a_physicsMaterial, a_pos);
	}
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
	m_pos = (*(vec3*)(&transform.p[0]));
	m_rot = quat(transform.q.w, transform.q.x, transform.q.y, transform.q.z);
	FBXModel::Update(dt);
}

void PhysModel::AttachRigidBodyConvex(float a_density, PhysScene* a_physics, PxMaterial* a_physicsMaterial, vec3 a_pos) {

	PxBoxGeometry box = PxBoxGeometry(1, 1, 1);
	PxTransform transform((*(PxVec3*)(&a_pos[0])));
	m_phys = PxCreateDynamic(*a_physics->m_physics, transform, box, *a_physicsMaterial, a_density);
	m_phys->userData = this; //link the PhysX actor to our FBX model
	int numberVerts = 0;

	//find out how many verts there are in total in model
	for (unsigned int i = 0; i < m_file->getMeshCount(); ++i) {
		FBXMeshNode* mesh = m_file->getMeshByIndex(i);
		numberVerts += mesh->m_vertices.size();
	}

	//reserve space for vert buffer
	PxVec3 *verts = new PxVec3[numberVerts];
	int vertIDX = 0;

	m_transform = glm::translate(m_pos) * glm::toMat4(m_rot) * glm::scale(m_scale) * m_modTransform;
	//copy our verts from all the sub meshes and tranform them into the same space
	for (unsigned int i = 0; i < m_file->getMeshCount(); ++i){
		FBXMeshNode* mesh = m_file->getMeshByIndex(i);
		int numberVerts = mesh->m_vertices.size();
		for (int vertCount = 0; vertCount< numberVerts; vertCount++){
			//Scale the global transform by an arbitrary number, based on what the model individually requires.
			glm::vec4 temp = mesh->m_globalTransform * glm::scale(vec3(m_physModelScale)) * mesh->m_vertices[vertCount].position;
			verts[vertIDX++] = PxVec3(temp.x, temp.y, temp.z);
		}
	}

	PxConvexMeshDesc convexDesc;
	convexDesc.points.count = numberVerts;
	convexDesc.points.stride = sizeof(PxVec3);
	convexDesc.points.data = verts;
	convexDesc.flags = PxConvexFlag::eCOMPUTE_CONVEX;
	convexDesc.vertexLimit = 128;
	PxDefaultMemoryOutputStream* buf = new PxDefaultMemoryOutputStream();
	
	if (a_physics->m_cooking != nullptr) {
		assert(a_physics->m_cooking->cookConvexMesh(convexDesc, *buf));
	}

	PxU8* contents = buf->getData();
	PxU32 size = buf->getSize();
	PxDefaultMemoryInputData input(contents, size);
	PxConvexMesh* convexMesh = a_physics->m_physics->createConvexMesh(input);
	PxTransform pose = PxTransform(PxVec3(0.0f, 0.0f, 0.0f));
	PxShape* convexShape = m_phys->createShape(PxConvexMeshGeometry(convexMesh), *a_physicsMaterial, pose);

	//remove the placeholder box we started with
	int numberShapes = m_phys->getNbShapes();
	PxShape** shapes = (PxShape**)_aligned_malloc(sizeof(PxShape*)*numberShapes, 16);
	m_phys->getShapes(shapes, numberShapes);
	m_phys->detachShape(**shapes);

	delete(verts); //delete our temporary vert buffer.

	a_physics->m_physicsScene->addActor(*m_phys);
}