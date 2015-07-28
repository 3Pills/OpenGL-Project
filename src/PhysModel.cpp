#include "PhysModel.h"
#include <Gizmos.h>
#include "PhysScene.h"
#include "Utility.h"

PhysModel::PhysModel(const char* a_szModelPath, vec3 a_pos, PxGeometry* a_geometry, PxMaterial* a_physicsMaterial, 
					 PhysScene* a_physics, float a_density, float a_roughness, float a_fresnelScale, vec3 a_scale, quat a_rot) 
		: FBXModel(a_szModelPath, a_pos, a_roughness, a_fresnelScale, a_scale, a_rot), m_density(a_density) {
	PxTransform transform(PxVec3(a_pos.x, a_pos.y, a_pos.z));
	m_geometry = new PxGeometryHolder();
	m_geometry->storeAny(*a_geometry); //Store geometry data.
	m_phys = PxCreateDynamic(*a_physics->m_physics, transform, m_geometry->any(), *a_physicsMaterial, m_density);
	a_physics->m_physicsScene->addActor(*m_phys);

	switch (m_geometry->getType()) {
	case PxGeometryType::eSPHERE:
		Gizmos::addSphere(m_pos, m_geometry->sphere().radius, 6, 6, vec4(1), &mat4(m_rot));
		break;
	case PxGeometryType::eCAPSULE:
		//Gizmos::addCylinderFilled(m_pos, m_geometry->sphere().radius, 6, 6, vec4(0, 1, 0, 1));
		break;
	case PxGeometryType::eBOX:
		PxVec3 extents = m_geometry->box().halfExtents;
		Gizmos::addAABB(m_pos, vec3(extents.x, extents.y, extents.z), vec4(1), &mat4(m_rot));
	}
}

PhysModel::~PhysModel() {
	FBXModel::~FBXModel();
	delete m_geometry;
}

void PhysModel::RenderDeferred(FlyCamera a_camera) {
	glUseProgram(m_deferredShader);

	int loc = glGetUniformLocation(m_deferredShader, "view");
	glUniformMatrix4fv(loc, 1, GL_FALSE, (float*)&a_camera.getView());

	loc = glGetUniformLocation(m_deferredShader, "projView");
	glUniformMatrix4fv(loc, 1, GL_FALSE, (float*)&a_camera.getProjectionView());

	if (m_file->getSkeletonCount() > 0) {
		loc = glGetUniformLocation(m_deferredShader, "bones");
		glUniformMatrix4fv(loc, m_skeleton->m_boneCount, GL_FALSE, (float*)m_skeleton->m_bones);
	}

	loc = glGetUniformLocation(m_deferredShader, "hasBones");
	glUniform1i(loc, (m_file->getSkeletonCount() > 0));

	loc = glGetUniformLocation(m_deferredShader, "camPos");
	glUniform3fv(loc, 1, (float*)&a_camera.getWorldTransform()[3].xyz);

	vec3 modelExtents = vec3(0);
	switch (m_geometry->getType()){
	case PxGeometryType::eSPHERE: {
		modelExtents = vec3(0, m_geometry->sphere().radius, 0);
		break;
	}
	case PxGeometryType::eBOX: {
		modelExtents = vec3(0, m_geometry->box().halfExtents.y, 0);
		break;
	}
	}
	mat4 centreOrigin = m_transform * (glm::translate(-modelExtents));

	loc = glGetUniformLocation(m_deferredShader, "transform");
	glUniformMatrix4fv(loc, 1, GL_FALSE, (float*)&centreOrigin);

	loc = glGetUniformLocation(m_deferredShader, "roughness");
	glUniform1f(loc, m_roughness);

	loc = glGetUniformLocation(m_deferredShader, "fresnelScale");
	glUniform1f(loc, m_fresnelScale);

	loc = glGetUniformLocation(m_deferredShader, "deferred");
	glUniform1i(loc, true);

	loc = glGetUniformLocation(m_deferredShader, "diffuse");
	glUniform1i(loc, 0);

	loc = glGetUniformLocation(m_deferredShader, "normal");
	glUniform1i(loc, 1);

	loc = glGetUniformLocation(m_deferredShader, "specular");
	glUniform1i(loc, 2);

	for (unsigned int i = 0; i < m_meshes.size(); ++i){
		FBXMeshNode* currMesh = m_file->getMeshByIndex(i);

		loc = glGetUniformLocation(m_deferredShader, "world");
		glUniformMatrix4fv(loc, 1, GL_FALSE, (float*)&currMesh->m_globalTransform);

		FBXMaterial* meshMat = currMesh->m_material;
		if (meshMat->textures[FBXMaterial::DiffuseTexture] != nullptr) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, meshMat->textures[FBXMaterial::DiffuseTexture]->handle);
		}
		if (meshMat->textures[FBXMaterial::NormalTexture] != nullptr) {
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, meshMat->textures[FBXMaterial::NormalTexture]->handle);
		}
		if (meshMat->textures[FBXMaterial::SpecularTexture] != nullptr) {
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, meshMat->textures[FBXMaterial::SpecularTexture]->handle);
		}

		glBindVertexArray(m_meshes[i].m_VAO);
		glDrawElements(GL_TRIANGLES, m_meshes[i].m_indexCount, GL_UNSIGNED_INT, 0);
	}
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