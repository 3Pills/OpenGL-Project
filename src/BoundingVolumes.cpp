#include "BoundingVolumes.h"

AABB::AABB(){ reset(); }
AABB::~AABB() {}

void AABB::reset() {
	m_vMin = m_vMax = vec3(1e37f);
}
void AABB::fit(const std::vector<glm::vec3>& a_points) {
	for (auto& p : a_points) {
		if (p.x < m_vMin.x) m_vMin.x = p.x;
		if (p.y < m_vMin.y) m_vMin.y = p.y;
		if (p.z < m_vMin.z) m_vMin.z = p.z;
		if (p.x > m_vMax.x) m_vMax.x = p.x;
		if (p.y > m_vMax.y) m_vMax.y = p.y;
		if (p.z > m_vMax.z) m_vMax.z = p.z;
	}
}

BoundingSphere::BoundingSphere() : m_vCentre(0), m_fRadius(0) { }
BoundingSphere::~BoundingSphere() {}

void BoundingSphere::fit(const std::vector<glm::vec3>& a_points) {
	vec3 min(1e37f), max(-1e37f);
	for (auto& p : a_points) {
		if (p.x < min.x) min.x = p.x;
		if (p.y < min.y) min.y = p.y;
		if (p.z < min.z) min.z = p.z;
		if (p.x > max.x) max.x = p.x;
		if (p.y > max.y) max.y = p.y;
		if (p.z > max.z) max.z = p.z;
	}
	m_vCentre = (min + max) * 0.5f;
	m_fRadius = glm::distance(min, m_vCentre);
}

AABB GenerateAABB(vec3* a_positions, const unsigned int a_count, const unsigned int a_stride = 0){
	AABB result = {};
	result.m_vMin = a_positions[0];
	result.m_vMax = a_positions[0];

	for (unsigned int i = 0; i < a_count; ++i){
		if (a_positions->x < result.m_vMin.x) result.m_vMin.x = a_positions->x;
		if (a_positions->y < result.m_vMin.y) result.m_vMin.y = a_positions->y;
		if (a_positions->z < result.m_vMin.z) result.m_vMin.z = a_positions->z;

		if (a_positions->x < result.m_vMax.x) result.m_vMax.x = a_positions->x;
		if (a_positions->y < result.m_vMax.y) result.m_vMax.y = a_positions->y;
		if (a_positions->z < result.m_vMax.z) result.m_vMax.z = a_positions->z;
	}
}

int ComparePlane(const vec4 a_plane, const AABB a_box) {
	vec3 plane_testA, plane_testB;
	vec3 normal = vec3(a_plane);

	plane_testA.x = (a_plane.x >= 0) ? a_box.m_vMin.x : a_box.m_vMax.x;
	plane_testA.y = (a_plane.y >= 0) ? a_box.m_vMin.y : a_box.m_vMax.y;
	plane_testA.z = (a_plane.z >= 0) ? a_box.m_vMin.z : a_box.m_vMax.z;
	plane_testB.x = (a_plane.x >= 0) ? a_box.m_vMax.x : a_box.m_vMin.x;
	plane_testB.y = (a_plane.y >= 0) ? a_box.m_vMax.y : a_box.m_vMin.y;
	plane_testB.z = (a_plane.z >= 0) ? a_box.m_vMax.z : a_box.m_vMin.z;

	float dA = glm::dot(vec3(a_plane), plane_testA) + a_plane.w;
	float dB = glm::dot(vec3(a_plane), plane_testB) + a_plane.w;

	return (dA > dB || dB < dA);
}