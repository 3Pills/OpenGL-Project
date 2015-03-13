#include "BoundingVolumes.h"
#include <Gizmos.h>

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

AABB GenerateAABB(vec3* a_positions, const unsigned int a_count, unsigned int a_stride) {
	AABB result = {};
	if (a_stride == 0) a_stride = sizeof(vec3);

	result.m_vMin = a_positions[0];
	result.m_vMax = a_positions[0];

	for (unsigned int i = 0; i < a_count; ++i){
		if (a_positions->x < result.m_vMin.x) result.m_vMin.x = a_positions->x;
		if (a_positions->y < result.m_vMin.y) result.m_vMin.y = a_positions->y;
		if (a_positions->z < result.m_vMin.z) result.m_vMin.z = a_positions->z;

		if (a_positions->x < result.m_vMax.x) result.m_vMax.x = a_positions->x;
		if (a_positions->y < result.m_vMax.y) result.m_vMax.y = a_positions->y;
		if (a_positions->z < result.m_vMax.z) result.m_vMax.z = a_positions->z;

		a_positions = (vec3*)((char*)a_positions + a_stride);
	}

	return result;
}

void RenderAABB(AABB a_aabb, mat4 transform)
{
	vec3 center = ((a_aabb.m_vMin + transform[3].xyz()) + (a_aabb.m_vMax + transform[3].xyz())) * 0.5f;
	vec3 extents = ((a_aabb.m_vMax + transform[3].xyz()) - (a_aabb.m_vMax + transform[3].xyz())) * 0.5f;

	Gizmos::addAABB(center, extents, vec4(1, 1, 1, 1), &transform);
}

bool OnPlanePositive(const vec4 a_plane, const AABB a_aabb, mat4 a_transform) {
	vec3 points[8];

	vec3 center = (a_aabb.m_vMin + a_aabb.m_vMax) * 0.5f;
	vec3 extents = (a_aabb.m_vMax - a_aabb.m_vMin) * 0.5f;

	for (unsigned int point = 0; point < 8; ++point) 
		points[point] = center;

	points[0] += vec3( extents.x,  extents.y,  extents.z);
	points[1] += vec3(-extents.x,  extents.y,  extents.z);
	points[2] += vec3(-extents.x, -extents.y,  extents.z);
	points[3] += vec3( extents.x, -extents.y,  extents.z);
	points[4] += vec3( extents.x,  extents.y, -extents.z);
	points[5] += vec3(-extents.x,  extents.y, -extents.z);
	points[6] += vec3(-extents.x, -extents.y, -extents.z);
	points[7] += vec3( extents.x, -extents.y, -extents.z);

	for (unsigned int point = 0; point < 8;	++point) {
		vec4 transformed_point = a_transform * vec4(points[point], 1);
		float d = glm::dot(a_plane.xyz(), transformed_point.xyz()) + a_plane.w;

		if (d > 0) return true;
	}

	return false;
}

bool OnPlanePositive(const vec4 a_plane, const BoundingSphere a_sphere, mat4 a_transform){
	return false;
}