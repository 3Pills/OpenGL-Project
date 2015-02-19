#include "Camera.h"
#include <GLFW\glfw3.h>

Camera::Camera():m_fFoV(glm::radians(90.0f)), m_fAspect(1280.0f/720.0f), m_fNearZ(0.1f), m_fFarZ(20000.0f), m_vEye(vec3(10,0,0)), m_vTo(vec3(0,0,0)), m_vUp(vec3(0,1,0)),
				 m_vMouseInitPos(vec2(0,0)){}

void Camera::UpdateProjectionViewTransform(){
	m_mViewTransform = glm::inverse(m_mWorldTransform);
	m_mProjTransform = glm::perspective(m_fFoV, m_fAspect, m_fNearZ, m_fFarZ);
	m_mProjViewTransform = m_mProjTransform * m_mViewTransform;
}

void Camera::setPerspective(const float a_fFoV, const float a_fAspect, const float a_fNearZ, const float a_fFarZ){
	m_fFoV = a_fFoV;
	m_fAspect = a_fAspect;
	m_fNearZ = a_fNearZ;
	m_fFarZ = a_fFarZ;
}

void Camera::setLookAt(const vec3 a_vFrom, const vec3 a_vTo, const vec3 a_vUp){
	m_vEye = a_vFrom;
	m_vTo = glm::normalize(a_vTo - a_vFrom);
	m_vUp = a_vUp;
}

void Camera::setPos(const vec3 a_vPos){
	m_mWorldTransform[3].x = a_vPos.x;
	m_mWorldTransform[3].y = a_vPos.y;
	m_mWorldTransform[3].z = a_vPos.z;
	m_vEye = a_vPos;
	m_vTo += a_vPos - m_vEye;
	m_mViewTransform = glm::inverse(m_mWorldTransform);
}

void Camera::setFoV(const float a_fFoV){
	m_fFoV = a_fFoV;
}

mat4 Camera::getWorldTransform(){
	return m_mWorldTransform;
}
mat4 Camera::getView(){
	return glm::inverse(m_mWorldTransform);
}
mat4 Camera::getProjection(){
	return m_mProjTransform;
}
mat4 Camera::getProjectionView(){
	return m_mProjViewTransform;
}

float Camera::getFoV(){
	return m_fFoV;
}

FlyCamera::FlyCamera(float a_fSpeed):m_fSpeed(a_fSpeed),Camera(){
	m_mWorldTransform[3] = vec4(0, 10, 10, 1);
}

void FlyCamera::setSpeed(float a_fSpeed){
	if (m_fSpeed != a_fSpeed){
		m_fSpeed = a_fSpeed;
	}
}

void FlyCamera::update(const float a_fdt){
	vec3 forward = (vec3)m_mWorldTransform[2];
	vec3 side = (vec3)m_mWorldTransform[0];
	GLFWwindow* curr_window = glfwGetCurrentContext();

	if (glfwGetKey(curr_window, GLFW_KEY_W)){
		m_mWorldTransform[3] -= m_mWorldTransform[2] * a_fdt * m_fSpeed;
	}
	if (glfwGetKey(curr_window, GLFW_KEY_S)){
		m_mWorldTransform[3] += m_mWorldTransform[2] * a_fdt * m_fSpeed;
	}
	if (glfwGetKey(curr_window, GLFW_KEY_A)){
		m_mWorldTransform[3] -= m_mWorldTransform[0] * a_fdt * m_fSpeed;
	}
	if (glfwGetKey(curr_window, GLFW_KEY_D)){
		m_mWorldTransform[3] += m_mWorldTransform[0] * a_fdt * m_fSpeed;
	}
	if (glfwGetKey(curr_window, GLFW_KEY_Q)){
		m_mWorldTransform[3] -= vec4(0, 1, 0, 0) * a_fdt * m_fSpeed;
	}
	if (glfwGetKey(curr_window, GLFW_KEY_E)){
		m_mWorldTransform[3] += vec4(0, 1, 0, 0) * a_fdt * m_fSpeed;
	}

	if (glfwGetKey(curr_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS){
		setSpeed(85);
	}
	else if (glfwGetKey(curr_window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS && glfwGetKey(curr_window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS){
		setSpeed(10);
	}
	else if (glfwGetKey(curr_window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS){
		setSpeed(20);
	}
	else{
		setSpeed(50);
	}

	if (glfwGetKey(curr_window, GLFW_KEY_SPACE) == GLFW_PRESS){
		if (m_bMousePress) {
			int width, height;
			glfwGetWindowSize(curr_window, &width, &height);
			m_vMouseInitPos = vec2(width / 2, height / 2);
			glfwSetInputMode(curr_window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
		}
		double x, y;
		glfwGetCursorPos(curr_window, &x, &y);
		glfwSetCursorPos(curr_window, m_vMouseInitPos.x, m_vMouseInitPos.y);
		if (!m_bMousePress) {
			x -= m_vMouseInitPos.x;
			y -= m_vMouseInitPos.y;
			
			x /= m_vMouseInitPos.x;
			y /= m_vMouseInitPos.y;
			
			x *= -1;
			y *= -1;

			mat4 yaw = glm::rotate((float)x, vec3(0, 1, 0));
			mat4 pitch = glm::rotate((float)y, side);
			mat4 rot = yaw * pitch;
			
			mat4 temp = m_mWorldTransform;
			m_mWorldTransform[0] = rot * m_mWorldTransform[0];
			m_mWorldTransform[1] = rot * m_mWorldTransform[1];
			m_mWorldTransform[2] = rot * m_mWorldTransform[2];
			if (m_mWorldTransform[1].y < 0) {
				m_mWorldTransform = temp;
				m_mWorldTransform[0] = yaw * m_mWorldTransform[0];
				m_mWorldTransform[1] = yaw * m_mWorldTransform[1];
				m_mWorldTransform[2] = yaw * m_mWorldTransform[2];
			}
		}
		m_bMousePress = false;
	}
	else{
		m_bMousePress = true;
		glfwSetInputMode(curr_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}

	UpdateProjectionViewTransform();
}