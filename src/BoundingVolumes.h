#ifndef _BOUNDING_VOLUMES_H_
#define _BOUNDING_VOLUMES_H_

#include "glm_header.h"
#include "Vertex.h"
#include <vector>

struct AABB {
	AABB();
	~AABB();

	void reset();
	void fit(const std::vector<glm::vec3>& a_points);
	vec3 m_vMin, m_vMax;
};

struct MeshObject{
	OpenGLData m_data;
	mat4 m_transform;
	AABB m_aabb;
};

struct BoundingSphere {
	BoundingSphere();
	~BoundingSphere();
	void fit(const std::vector<glm::vec3>& a_points);
	vec3 m_vCentre;
	float m_fRadius;
};

bool OnPlanePositive(const vec4 a_plane, const AABB a_aabb, mat4 a_transform);
bool OnPlanePositive(const vec4 a_plane, const BoundingSphere a_sphere, mat4 a_transform);

AABB GenerateAABB(vec3* a_positions, const unsigned int a_count, unsigned int a_stride = 0);
void RenderAABB(AABB aabb, mat4 transform);
#endif//_BOUNDING_VOLUMES_H_