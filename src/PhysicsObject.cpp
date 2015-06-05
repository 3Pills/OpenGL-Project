#include "PhysicsObject.h"
#include <math.h>
#include <Gizmos.h>

DIYRigidBody::DIYRigidBody(vec2 a_pos, vec2 a_vel, float a_rot, float a_mass) 
: m_pos(a_pos), m_vel(a_vel), m_rot(a_rot), m_mass(a_mass)
{}
void DIYRigidBody::update(vec2 a_gravity, float a_timeStep){
	m_vel += 0.5 * a_gravity * a_timeStep;
}
void DIYRigidBody::applyForce(vec2 force){
	m_vel += force;
}
void DIYRigidBody::applyForceToActor(DIYRigidBody* otherActor, vec2 force){
	otherActor->m_vel += force;
	m_vel += -force;
}
void DIYRigidBody::debug(){

}

SphereClass::SphereClass(vec2 a_pos, vec2 a_vel, float a_mass, float a_radius, vec4 a_colour)
: DIYRigidBody(m_pos, m_vel, 0, a_mass), m_radius(a_radius), m_colour(a_colour)
{}
void SphereClass::makeGizmo(){
	Gizmos::add2DCircle(m_pos, m_radius, 24, m_colour);
}

void DIYPhysicsScene::update(){
	for (int i = 0; i < m_actors.size(); i++) {
		m_actors[i]->update(m_gravity, m_timeStep);
	}
}
void DIYPhysicsScene::debugScene(){

}
void DIYPhysicsScene::addActor(PhysicsObject* a_actor){
	m_actors.push_back(a_actor);
}
void DIYPhysicsScene::removeActor(PhysicsObject* a_actor){
	for (int i = 0; i < m_actors.size(); i++) {
		if (m_actors[i]->m_shapeID == a_actor->m_shapeID) {
			m_actors.erase(m_actors.begin() + i);
			break;
		}
	}
}
void DIYPhysicsScene::addGizmos(){

}	