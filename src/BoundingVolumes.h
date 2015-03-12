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
	AABB m_aabb;
};

struct BoundingSphere {
	BoundingSphere();
	~BoundingSphere();
	void fit(const std::vector<glm::vec3>& a_points);
	vec3 m_vCentre;
	float m_fRadius;
};

int ComparePlane(const vec4 a_plane, const AABB a_sphere);
int ComparePlane(const vec4 a_plane, const BoundingSphere a_sphere);

AABB GenerateAABB(vec3* a_positions, const unsigned int a_count, const unsigned int a_stride = 0);
#endif//_BOUNDING_VOLUMES_H_