#ifndef COLLISION_SHAPE_HIERARCHES_H_
#define COLLISION_SHAPE_HIERARCHES_H_
#include "Application.h"
#include "Camera.h"

#include <PxPhysicsAPI.h>
#include <PxScene.h>
#include <pvd/PxVisualDebugger.h>

#include "tiny_obj_loader.h"
#include "Vertex.h"
#include "AntTweakBar.h"

using namespace physx;

class CollisionShapeHierarchies : public Application
{
	FlyCamera m_oCamera;
	std::vector<OpenGLData> m_meshes;

	PxFoundation* m_physics_foundation;
	PxPhysics* m_physics;
	PxScene* m_physics_scene;

	PxDefaultErrorCallback m_default_error_callback;
	PxDefaultAllocator m_default_allocator;
	PxSimulationFilterShader m_default_filter_shader;

	PxMaterial* m_physics_material;
	PxMaterial* m_box_material[3];
	PxCooking* m_physics_cooker;

	std::vector<PxRigidActor*> m_physActors;

	unsigned int m_programID;
public:
	CollisionShapeHierarchies();
	virtual ~CollisionShapeHierarchies();

	virtual bool startup();
	virtual bool shutdown();

	virtual bool update();
	virtual void draw();

	void setupPhysx();
	void setupTutorial1();
	void setupVisualDebugger();

	void CreateOpenGLBuffers(std::vector<tinyobj::shape_t>& shapes);
	void CleanOpenGLBuffers();
};

#endif//COLLISION_SHAPE_HIERARCHES_H_