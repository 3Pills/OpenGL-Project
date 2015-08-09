#include "Entity.h"
#include "FBXModel.h"
#include <PhysX.h>
#include "PhysScene.h"
#include "Utility.h"

Entity::Entity(const vec3 a_pos, const float a_maxSpeed, FBXModel* a_model) : m_velocity(vec3(0)), m_maxSpeed(a_maxSpeed), m_grounded(false) {
	m_controller = PhysScene::AddPlayerController(PxExtendedVec3(a_pos.x, a_pos.y, a_pos.z), PhysScene::m_physics->createMaterial(0.5f, 0.5f, 0.5f), a_model);
}
Entity::~Entity() {
	m_controller->release();
}

void Entity::Update(const float dt) {
	//Velocity falloff
	m_velocity = vec3(m_velocity.x * dt * 30.f, m_velocity.y, m_velocity.z * dt * 30.f);

	FBXModel* playerModel = (FBXModel*)m_controller->getActor()->userData;

	vec3 flatVelocity = vec3(m_velocity.x, 0, m_velocity.z);

	//Rotate the entity model if the controller is moving fast enough.
	if (glm::length(flatVelocity) > 0.001f) {
		playerModel->m_rot = conjugate(glm::toQuat(glm::lookAt(vec3(0), -flatVelocity, vec3(0, 1, 0))));
	}

	//Move the controller.
	m_controller->move(PxVec3(m_velocity.x, m_velocity.y, m_velocity.z), 0.001f, dt, PxControllerFilters());
}

vec3 Entity::GetPos(){
	return vec3(m_controller->getPosition().x, m_controller->getPosition().y, m_controller->getPosition().z);
}
PxController* Entity::GetController() {
	return m_controller;
}

Player::Player(const vec3 a_pos, const float a_maxSpeed, FBXModel* a_model) : Entity(a_pos, a_maxSpeed, a_model) {}

void Player::Update(const float dt, mat4 a_camTransform) {
	//Access the FBXModel class of the player, stored as userData within the actor
	FBXModel* playerModel = (FBXModel*)m_controller->getActor()->userData;

	GLFWwindow* window = glfwGetCurrentContext();

	//scan the keys and set up our intended velocity
	vec3 velocity(0);
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
		velocity.z -= 1;
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		velocity.z += 1;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
		velocity.x -= 1;
	}
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		velocity.x += 1;
	}
	//Normalize velocity vector, to stop diagonals getting more speed.
	if (velocity != vec3(0)) {
		glm::normalize(velocity);
		playerModel->SetAnimation(eRUN);
	}
	else {
		playerModel->SetAnimation(eIDLE);
	}
	velocity *= m_maxSpeed * dt;

	//Make controls relative to camera direction.
	//Flatten the camera foward and player positions so a camera looking up or down will not affect the final velocity.
	vec3 camForward = vec3(a_camTransform[2].x, 0, a_camTransform[2].z);
	vec3 playerFlat = vec3(m_controller->getPosition().x, 0, m_controller->getPosition().z);

	//Create a rotation quaternion to rotate the velocity in the direction of the camera forward.
	quat cameraRot = glm::conjugate(glm::toQuat(glm::lookAt(playerFlat, playerFlat - camForward, vec3(0, 1, 0))));
	vec3 rotVelocity = cameraRot * velocity;

	m_velocity += rotVelocity;

	//Create a flattened movement direction for ground movement.
	vec3 moveDir = vec3(m_velocity.x, 0, m_velocity.z);
	m_velocity = vec3(moveDir.x, m_velocity.y, moveDir.z);


	//Access the hitreport, stored as userdata within the controller.
	PlayerHitReport* hitReport = (PlayerHitReport*)m_controller->getUserData();
	//check if we have a contact normal. if y is greater than .3 we assume this is solid ground.This is a rather primitive way to do this.Can you do better ?
	if (hitReport->getPlayerContactNormal().y > .3f) {
		m_velocity.y = -0.1f;
		if (glfwGetKey(window, GLFW_KEY_SPACE)) {
			m_velocity.y = 30.f * dt;
		}
		m_grounded = true;
	}
	else {
		m_velocity.y += -1.5f * dt;
		m_grounded = false;
	}
	hitReport->clearPlayerContactNormal();

	Entity::Update(dt);

}

AIEntity::AIEntity(const vec3 a_pos, const float a_maxSpeed, AINode* a_targetNode, FBXModel* a_model) 
	: Entity(a_pos, a_maxSpeed, a_model), m_targetNode(a_targetNode), m_raycastBuffer(PxRaycastBuffer(m_hits, 20)),
	m_aggroRadius(30.0f), m_goalRadius(10.0f)
{}

void AIEntity::Update(const float dt, const vec3 a_playerPos) {
	//Swap to the next node if we get close enough to the target Node.
	if (glm::length(GetPos() - m_targetNode->m_pos) < m_goalRadius) {
		m_targetNode = m_targetNode->m_next;
	}

	if (glm::length(GetPos() - a_playerPos) < m_aggroRadius) {
		m_target = a_playerPos;
	}
	else {
		m_target = m_targetNode->m_pos;
	}

	vec3 targetFlat = vec3(m_target.x, 0, m_target.z);
	vec3 posFlat = vec3(GetPos().x, 0, GetPos().z);
	vec3 flatVelocity = targetFlat - posFlat;

	m_velocity += (glm::length(flatVelocity) > 0) ? glm::normalize(flatVelocity) * dt * m_maxSpeed : vec3(0);

	PxVec3 position(GetPos().x, GetPos().y, GetPos().z);

	bool status = PhysScene::m_physicsScene->raycast(position, (PxVec3(m_target.x, m_target.y, m_target.z) - position).getNormalized(), 5.0f, m_raycastBuffer);
	if (status) {
		if (m_raycastBuffer.hasBlock) {
			PxVec3 steerDirection = m_raycastBuffer.block.normal.cross(PxVec3(0, 1, 0));
			m_velocity += vec3(steerDirection.x, steerDirection.y, steerDirection.z) * dt * m_maxSpeed;
		}
	}

	//Access the hitreport, stored as userdata within the controller. 
	PlayerHitReport* hitReport = (PlayerHitReport*)m_controller->getUserData();

	if (hitReport->getPlayerContactNormal().y > .3f) {
		m_velocity.y = -0.1f;
		m_grounded = true;
	}
	else {
		m_velocity.y += -1.5f * dt;
		m_grounded = false;
	}
	hitReport->clearPlayerContactNormal();

	Entity::Update(dt);
}

void AIEntity::SetTarget(const vec3 a_target) { m_target = a_target; }
vec3 AIEntity::GetTarget() { return m_target; }

Navi::Navi(PxRigidActor* a_parent) 
	: m_parent(a_parent), m_target(a_parent), m_pos(vec3(0)), m_timer(0),
	m_sprite(m_pos, 2, 3, vec3(1), vec3(1), "./data/textures/particles/glow3.png"), 
	m_light(m_pos, vec3(1), 15),
	m_searchRadius(PxSphereGeometry(30.0f)),
	m_overlapBuffer(PxOverlapBuffer(m_hits, 20)) {}

Navi::~Navi() {
	m_sprite.m_frequency = 0.5f;
}

void Navi::Update(const float dt, const mat4 a_camTransform, const vec3 a_playerPos){
	m_timer += dt;

	//Move the sphere a bit off, because it's not centred.
	PxVec3 currPlayerPos = PxVec3(a_playerPos.x, a_playerPos.y + 5.0f, a_playerPos.z);

	//Search around the player for any nearby objects, and hover over them.
	bool status = PhysScene::m_physicsScene->overlap(m_searchRadius, PxTransform(currPlayerPos), m_overlapBuffer, PxQueryFilterData(PxQueryFlag::eDYNAMIC));
	if (status) {
		//If we have dynamic rigidbodies in range, check if the player object is the only one.
		bool onlyParent = true;
		for (unsigned int i = 0; i < m_overlapBuffer.getNbTouches(); i++) {
			if (m_overlapBuffer.getTouch(i).actor != m_parent) {
				m_target = m_overlapBuffer.getTouch(i).actor;
				onlyParent = false;
				break;
			}
		}
		//If we dont find any other objects, set target to the parent.
		if (onlyParent) {
			m_target = m_parent;
		}
	}
	else {
		m_target = m_parent;
	}

	PxRigidDynamic* actor = m_target->isRigidDynamic();

	if (actor) {
		PxTransform tTransform = actor->getGlobalPose();
		//Get the position of the target actor
		vec3 position = vec3(tTransform.p.x, tTransform.p.y, tTransform.p.z);

		//Circle around the target
		vec3 offset = vec3(cos(m_timer / 2) * 6, (cos(m_timer * 4) / 2) + 9, sin(m_timer / 2) * 6);

		m_pos = glm::lerp(m_pos, position + offset, 0.05f);
	}

	if (m_target == m_parent) {
		m_light.m_color = vec3(1);
		m_sprite.m_maxColor = vec3(1);
		m_sprite.m_minColor = vec3(1);
	}
	else if (actor->getRigidDynamicFlags() & PxRigidDynamicFlag::eKINEMATIC) {
		m_light.m_color = vec3(1, 1, 0);
		m_sprite.m_maxColor = vec3(1, 1, 0);
		m_sprite.m_minColor = vec3(0.8f, 0.8f, 0);
	}
	else {
		m_light.m_color = vec3(0, 1, 0);
		m_sprite.m_maxColor = vec3(0, 1, 0);
		m_sprite.m_minColor = vec3(0, 0.8f, 0);
	}

	m_light.m_pos = m_pos;
	m_sprite.m_pos = m_pos;
	m_sprite.Update(dt, a_camTransform);
}

void Navi::RenderLight(Camera* a_camera, const unsigned int a_program){
	m_light.Render(a_camera, a_program);
}

void Navi::RenderSprite(Camera* a_camera){
	m_sprite.Render(a_camera);
}

vec3 Navi::GetPos() {
	return m_pos;
}

vec3 Navi::GetColor() {
	return m_sprite.m_color;
}