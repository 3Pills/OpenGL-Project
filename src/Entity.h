#ifndef ENTITY_H_
#define ENTITY_H_
#include <PhysX.h>
#include "Camera.h"
#include "Vertex.h"
#include "Lights.h"

class FBXModel;

class Entity {
protected:
	PxController* m_controller; //Entity's PhysX Controller
	vec3 m_velocity; //Entity's velocity
	float m_maxSpeed; //Entity's speed
	bool m_grounded; //bool for grounded entity
public:
	Entity(const vec3 a_pos, const float a_maxSpeed, FBXModel* a_model);
	~Entity();

	virtual void Update(const float dt);
	vec3 GetPos();
	PxController* GetController();
};

class Player : public Entity {
public:
	Player(const vec3 a_pos, const float a_maxSpeed, FBXModel* a_model);
	virtual void Update(const float dt, const mat4 a_camTransform);
};

//Node that the AIEntity class will patrol through
struct AINode {
	vec3 m_pos;
	AINode* m_prev;
	AINode* m_next;
};

//AI class that patrols the world and follow the player if they get close.
class AIEntity : public Entity {
	AINode* m_targetNode; //The next node the ai is to move towards
	vec3 m_target; //An override target that will be set should the AI stray from its path

	float m_aggroRadius; //radius player must enter to deter AI from its path.
	float m_goalRadius; //radius ai must enter to consider being 'at' a node

	PxRaycastHit m_hits[20];
	PxRaycastBuffer m_raycastBuffer;

public:
	AIEntity(const vec3 a_pos, const float a_maxSpeed, AINode* a_targetNode, FBXModel* a_model);

	void SetTarget(const vec3 a_target);
	vec3 GetTarget();

	virtual void Update(const float dt, const vec3 a_playerPos);
};

//AI class designed to follow player and highlight points of interest.
class Navi {
	PxRigidActor* m_parent;
	PxRigidActor* m_target;

	PxSphereGeometry m_searchRadius;

	PxOverlapHit m_hits[20];
	PxOverlapBuffer m_overlapBuffer;

	PointLight m_light;
	Sprite m_sprite;

	vec3 m_pos;

	float m_timer;
public:
	Navi(PxRigidActor* a_parent);
	~Navi();
	virtual void Update(const float dt, const mat4 a_camTransform, const vec3 a_playerPos);
	void RenderLight(Camera* a_camera, const unsigned int a_program);
	void RenderSprite(Camera* a_camera);

	void SetTarget(const vec3 a_target);
	vec3 GetTarget();

	vec3 GetPos();
	vec3 GetColor();
};

#endif//ENTITY_H_