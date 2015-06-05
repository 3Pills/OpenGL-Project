#ifndef PHYSICS_OBJECT_H_
#define PHYSICS_OBJECT_H_

#include "glm_header.h"
#include "Utility.h"
#include <vector>

enum ShapeType {
	PLANE = 0,
	SPHERE = 1,
	BOX = 2,
};

struct PhysicsObject {
	ShapeType m_shapeID;
	virtual void update(vec2 gravity, float timeStep) = 0;
	virtual void debug() = 0;
	virtual void makeGizmo() = 0;
	virtual void resetPos(){};
};

struct DIYRigidBody : public PhysicsObject {
	vec2 m_pos;
	vec2 m_vel;
	float m_mass;
	float m_rot;
	DIYRigidBody(vec2 a_pos, vec2 a_vel, float a_rot, float a_mass);
	virtual void update(vec2 gravity, float timeStep);
	virtual void debug();
	void applyForce(vec2 force);
	void applyForceToActor(DIYRigidBody* otherActor, vec2 force);
};

struct SphereClass : public DIYRigidBody {
	float m_radius;
	vec4 m_colour;
	SphereClass(vec2 a_pos, vec2 a_vel, float a_mass, float a_radius, vec4 a_colour);
	virtual void makeGizmo();
};

struct DIYPhysicsScene {
	vec2 m_gravity;
	float m_timeStep;
	std::vector<PhysicsObject*> m_actors;
	void addActor(PhysicsObject* a_actor);
	void removeActor(PhysicsObject* a_actor);
	void update();
	void debugScene();
	void addGizmos();
};
#endif//PHYSICS_OBJECT_H_