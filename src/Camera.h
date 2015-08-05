#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "glm_header.h"

class Camera {
protected:
	int m_width, m_height;
	float m_FoV, m_aspect, m_nearZ, m_farZ;
	vec2 m_mouseInitPos;
	mat4 m_worldTransform, m_viewTransform, m_projTransform, m_projViewTransform;
	void UpdateProjView();
public:
	vec3 m_pos, m_viewTarget, m_up;
	mat4 m_yaw, m_pitch;
	quat m_rot;
	Camera();
	virtual void Update(const float dt) = 0;
	void SetPerspective(const float a_FoV, const float a_aspect, const float a_nearZ, const float a_farZ);

	void SetPos(const vec3 a_pos);
	void SetLookAt(const vec3 a_target);

	void SetFoV(const float a_FoV);
	vec3 PickAgainstPlane(float x, float y, vec4 a_plane);
	mat4 GetWorldTransform();
	mat4 GetView();
	mat4 GetProjection();
	mat4 GetProjectionView();
	void GetFrustumPlanes(vec4* a_planes);
	float GetFoV();
	float GetAspectRatio();
	float GetNearZ();
	float GetFarZ();
};

class FlyCamera : public Camera {
private:
	bool m_mousePress;
	float m_speed;
public:
	FlyCamera(const float a_speed);
	virtual void Update(const float dt);

	void SetSpeed(const float a_speed);
	float GetSpeed();
};

class TargetCamera : public FlyCamera {
private:
	float m_innerRadius, m_outerRadius, m_lerpSpeed;
public:
	bool m_locked;
	vec3 m_offset;
	mat4 m_targetTransform;

	TargetCamera(const float a_speed);
	virtual void Update(const float dt, const vec3 m_target);

	void TweenPos(const vec3 a_pos);
	void TweenLookAt(const vec3 a_target);
	void SetLerpSpeed(const float a_lerpSpeed);
	float GetLerpSpeed();
};

#endif//_CAMERA_H_