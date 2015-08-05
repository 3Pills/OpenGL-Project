#include "Camera.h"
#include <GLFW\glfw3.h>

Camera::Camera() : m_FoV(glm::radians(90.0f)), m_aspect(1280.0f / 720.0f), m_nearZ(0.1f), m_farZ(20000.0f), m_pos(vec3(0, 10, 10)), m_viewTarget(vec3(0, -10, -10)), m_up(vec3(0, 1, 0)),
				   m_mouseInitPos(vec2(0)){
	SetPos(m_pos);
	SetLookAt(m_viewTarget);
	GLFWwindow* curr_window = glfwGetCurrentContext();
	glfwGetWindowSize(curr_window, &m_width, &m_height);
	UpdateProjView();
}

void Camera::Update(float dt) {
	GLFWwindow* curr_window = glfwGetCurrentContext();
	int width = 0, height = 0;
	glfwGetWindowSize(curr_window, &width, &height);
	if (width != m_width || height != m_height) {
		m_width = width;
		m_height = height;
		SetPerspective(m_FoV, (m_width / m_height) >= 1 ? ((float)m_width / m_height) : ((float)m_height / m_width), m_nearZ, m_farZ);
	}
	UpdateProjView();
}

void Camera::UpdateProjView(){
	m_worldTransform = glm::translate(m_pos) * glm::toMat4(m_rot);
	m_viewTransform = glm::inverse(m_worldTransform);
	m_projTransform = glm::perspective(m_FoV, m_aspect, m_nearZ, m_farZ);
	m_projViewTransform = m_projTransform * m_viewTransform;
}

void Camera::SetPerspective(const float a_FoV, const float a_aspect, const float a_nearZ, const float a_farZ){
	m_FoV = a_FoV;
	m_aspect = a_aspect;
	m_nearZ = a_nearZ;
	m_farZ = a_farZ;
}

void Camera::SetLookAt(const vec3 a_viewTarget){
	m_rot = glm::conjugate(glm::toQuat(glm::lookAt(m_pos, a_viewTarget, m_up)));
}

void Camera::SetPos(const vec3 a_pos){
	m_pos = a_pos;
}

void Camera::SetFoV(const float a_FoV){
	m_FoV = a_FoV;
}

vec3 Camera::PickAgainstPlane(float x, float y, vec4 plane) {
	float nxPos = x / m_width;
	float nyPos = y / m_height;

	float sxPos = nxPos - 0.5f;
	float syPos = nyPos - 0.5f;

	float fxPos = sxPos * 2;
	float fyPos = syPos * 2;

	mat4 invViewProj = glm::inverse(m_projViewTransform);

	vec4 mousePos(fxPos, fyPos, 1, 1);
	vec4 worldPos = invViewProj * mousePos;

	worldPos /= worldPos.w;

	vec3 camPos = m_worldTransform[3].xyz();
	vec3 dir = worldPos.xyz() - camPos;

	float t = -(glm::dot(camPos, plane.xyz()) + plane.w) / (glm::dot(dir, plane.xyz()));

	return(camPos + dir * t);
}

mat4 Camera::GetWorldTransform(){
	return m_worldTransform;
}
mat4 Camera::GetView(){
	return glm::inverse(m_worldTransform);
}
mat4 Camera::GetProjection(){
	return m_projTransform;
}
mat4 Camera::GetProjectionView(){
	return m_projViewTransform;
}
void Camera::GetFrustumPlanes(vec4* planes){
	// right side
	planes[0] = vec4(m_projViewTransform[0][3] - m_projViewTransform[1][0], 
					 m_projViewTransform[1][3] - m_projViewTransform[1][0],
					 m_projViewTransform[2][3] - m_projViewTransform[2][0], 
					 m_projViewTransform[3][3] - m_projViewTransform[3][0]);
	// left side
	planes[1] = vec4(m_projViewTransform[0][3] + m_projViewTransform[0][0], 
					 m_projViewTransform[1][3] + m_projViewTransform[1][0],
					 m_projViewTransform[2][3] + m_projViewTransform[2][0], 
					 m_projViewTransform[3][3] + m_projViewTransform[3][0]);
	// top
	planes[2] = vec4(m_projViewTransform[0][3] - m_projViewTransform[0][1], 
					 m_projViewTransform[1][3] - m_projViewTransform[1][1],
					 m_projViewTransform[2][3] - m_projViewTransform[2][1], 
					 m_projViewTransform[3][3] - m_projViewTransform[3][1]);
	// bottom
	planes[3] = vec4(m_projViewTransform[0][3] + m_projViewTransform[0][1], 
					 m_projViewTransform[1][3] + m_projViewTransform[1][1],
					 m_projViewTransform[2][3] + m_projViewTransform[2][1], 
					 m_projViewTransform[3][3] + m_projViewTransform[3][1]);
	// far
	planes[4] = vec4(m_projViewTransform[0][3] - m_projViewTransform[0][2], 
					 m_projViewTransform[1][3] - m_projViewTransform[1][2],
					 m_projViewTransform[2][3] - m_projViewTransform[2][2], 
					 m_projViewTransform[3][3] - m_projViewTransform[3][2]);
	// near
	planes[5] = vec4(m_projViewTransform[0][3] + m_projViewTransform[0][2], 
					 m_projViewTransform[1][3] + m_projViewTransform[1][2], 
					 m_projViewTransform[2][3] + m_projViewTransform[2][2], 
					 m_projViewTransform[3][3] + m_projViewTransform[3][2]);

	for (int i = 0; i < 6; i++)
		planes[i] = glm::normalize(planes[i]);
}

float Camera::GetFoV(){
	return m_FoV;
}
float Camera::GetAspectRatio(){
	return m_aspect;
}
float Camera::GetNearZ(){
	return m_nearZ;
}
float Camera::GetFarZ(){
	return m_farZ;
}

FlyCamera::FlyCamera(float a_speed) : Camera(), m_speed(a_speed){}
void  FlyCamera::SetSpeed(const float a_speed) { m_speed = a_speed; }
float FlyCamera::GetSpeed() { return m_speed; }

void FlyCamera::Update(const float dt){
	vec3 side = (vec3)m_worldTransform[0];
	GLFWwindow* curr_window = glfwGetCurrentContext();

	if (glfwGetKey(curr_window, GLFW_KEY_W))
		m_worldTransform[3] -= m_worldTransform[2] * m_speed * dt;
	if (glfwGetKey(curr_window, GLFW_KEY_S))
		m_worldTransform[3] += m_worldTransform[2] * m_speed * dt;
	if (glfwGetKey(curr_window, GLFW_KEY_A))
		m_worldTransform[3] -= m_worldTransform[0] * m_speed * dt;
	if (glfwGetKey(curr_window, GLFW_KEY_D))
		m_worldTransform[3] += m_worldTransform[0] * m_speed * dt;
	if (glfwGetKey(curr_window, GLFW_KEY_Q))
		m_worldTransform[3] -= vec4(0, 1, 0, 0) * m_speed * dt;
	if (glfwGetKey(curr_window, GLFW_KEY_E))
		m_worldTransform[3] += vec4(0, 1, 0, 0) * m_speed * dt;

	m_pos = m_worldTransform[3].xyz;

	if (glfwGetKey(curr_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS){
		m_speed = 85;
	}
	else if (glfwGetKey(curr_window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS && glfwGetKey(curr_window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS){
		m_speed = 10;
	}
	else if (glfwGetKey(curr_window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS){
		m_speed = 20;
	}
	else{
		m_speed = 50;
	}
	if (glfwGetMouseButton(curr_window, GLFW_MOUSE_BUTTON_2) == GLFW_PRESS){
		if (m_mousePress) {
			int width, height;
			glfwGetWindowSize(curr_window, &width, &height);
			m_mouseInitPos = vec2(width / 2, height / 2);
			glfwSetInputMode(curr_window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
		}
		double x, y;
		glfwGetCursorPos(curr_window, &x, &y);
		glfwSetCursorPos(curr_window, m_mouseInitPos.x, m_mouseInitPos.y);
		if (!m_mousePress) {
			x -= m_mouseInitPos.x;
			y -= m_mouseInitPos.y;
			
			x /= m_mouseInitPos.x;
			y /= m_mouseInitPos.y;
			
			x *= -1;
			y *= -1;

			if (glfwGetKey(curr_window, GLFW_KEY_UP))
				y += 0.025;
			if (glfwGetKey(curr_window, GLFW_KEY_DOWN))
				y -= 0.025;
			if (glfwGetKey(curr_window, GLFW_KEY_LEFT))
				x += 0.025;
			if (glfwGetKey(curr_window, GLFW_KEY_RIGHT))
				x -= 0.025;

			m_yaw = glm::rotate((float)x, m_up);
			m_pitch = glm::rotate((float)y, side);

			mat4 pitchClamp = m_yaw * m_pitch * m_worldTransform;
			if (pitchClamp[1].y > 0) {
				m_rot = glm::toQuat(m_yaw * m_pitch) * m_rot;
			}
			else {
				m_rot = glm::toQuat(m_yaw) * m_rot;
			}
			
			/*mat4 temp = m_worldTransform;
			m_worldTransform[0] = rot * m_worldTransform[0];
			m_worldTransform[1] = rot * m_worldTransform[1];
			m_worldTransform[2] = rot * m_worldTransform[2];
			if (m_worldTransform[1].y < 0) {
				m_worldTransform = temp;
				m_worldTransform[0] = yaw * m_worldTransform[0];
				m_worldTransform[1] = yaw * m_worldTransform[1];
				m_worldTransform[2] = yaw * m_worldTransform[2];
			}*/
		}
		m_mousePress = false;
	}
	else{
		m_mousePress = true;
		glfwSetInputMode(curr_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
	Camera::Update(dt);
}

TargetCamera::TargetCamera(const float a_speed) : FlyCamera(a_speed), m_locked(true), m_innerRadius(25.f), m_outerRadius(45.f), m_lerpSpeed(0.05f) {}

void TargetCamera::TweenPos(const vec3 a_pos) {
	m_pos = glm::lerp(m_pos, a_pos, m_lerpSpeed);
}
void TargetCamera::TweenLookAt(const vec3 a_target) {
	m_rot = glm::slerp(m_rot, glm::conjugate(glm::toQuat(glm::lookAt(m_pos, a_target, m_up))), m_lerpSpeed);
}

void  TargetCamera::SetLerpSpeed(const float a_lerpSpeed) { m_lerpSpeed = a_lerpSpeed; }
float TargetCamera::GetLerpSpeed() { return m_lerpSpeed; }

void TargetCamera::Update(const float dt, const vec3 m_target) {
	if (!m_locked) {
		FlyCamera::Update(dt);
		return;
	}

	vec3 target = m_target + vec3(0, 5, 0);

	float dist = glm::length(target - m_pos);

	vec3 behind = (-vec3((target - m_pos).x, 0, (target - m_pos).z) * 10);
	behind.y += 10;
	
	if (dist > m_outerRadius) {
		m_offset.z = m_outerRadius;
		TweenPos(target + glm::normalize(behind) * m_outerRadius);
	}
	else if (dist < m_innerRadius) {
		m_offset.z = m_innerRadius;
		TweenPos(target + glm::normalize(behind) * m_innerRadius);
	}
	TweenLookAt(target);

	//GLFWwindow* window = glfwGetCurrentContext();

	//vec3 newOffset = vec3(0, 0, m_offset.z);
	//if (glfwGetKey(window, GLFW_KEY_UP))
	//	newOffset.y += 0.25;
	//if (glfwGetKey(window, GLFW_KEY_DOWN))
	//	newOffset.y -= 0.25;
	//if (glfwGetKey(window, GLFW_KEY_LEFT))
	//	newOffset.x -= 0.25;
	//if (glfwGetKey(window, GLFW_KEY_RIGHT))
	//	newOffset.x += 0.25;
	//m_offset = m_rot * newOffset;
	//SetLookAt(m_targetTransform[3].xyz + vec3(0, 10, 0));

	UpdateProjView();
}