#include "PhysScene.h"

#define Assert(val) if (val){}else{ *((char*)0) = 0;}
#define ArrayCount(val) (sizeof(val)/sizeof(val[0]))

class MyAllocator : public physx::PxAllocatorCallback{
	virtual void* allocate(size_t size, const char* typeName, const char* filename, int line){
		void* new_mem = malloc(size + 32);
		void* result = (char*)new_mem + (16 - ((size_t)new_mem % 16));
		Assert(((size_t)result % 16) == 0);
		*(void**)result = new_mem;
		return (char*)result + 16;
	}

	virtual void deallocate(void* ptr){
		if (ptr){
			void* real_ptr = *(void**)((char*)ptr - 16);
			free(real_ptr);
		}
	}
};

PhysScene::PhysScene() {
	PxAllocatorCallback *myCallback = new MyAllocator();

	m_physicsFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, *myCallback, m_defaultErrorCallback);
	m_physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_physicsFoundation, PxTolerancesScale());

	PxInitExtensions(*m_physics);

	PxSceneDesc sceneDesc(m_physics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0, -20.0f, 0);
	sceneDesc.filterShader = &PxDefaultSimulationFilterShader;
	sceneDesc.cpuDispatcher = PxDefaultCpuDispatcherCreate(1);

	m_physicsScene = m_physics->createScene(sceneDesc);

	PxTolerancesScale toleranceScale;
	toleranceScale.mass = 1000;
	toleranceScale.speed = sceneDesc.gravity.y;
	bool value = toleranceScale.isValid(); // make sure this value is always true

	if (value) {
		m_cooking = PxCreateCooking(PX_PHYSICS_VERSION, *m_physicsFoundation, PxCookingParams(toleranceScale));
	}

	//Create a plane below everything to catch stray physics objects.
	m_planePose = PxTransform(PxVec3(0.f, 0.f, 0.f), PxQuat(PxHalfPi * 1.0f, PxVec3(0.0f, 0.0f, 1.0f)));
	m_plane = PxCreateStatic(*m_physics, m_planePose, PxPlaneGeometry(), *m_physics->createMaterial(1.f, 1.f, 1.f));
	m_physicsScene->addActor(*m_plane);

	if (m_physics->getPvdConnectionManager() != NULL)
		m_debuggerConnection = PxVisualDebuggerExt::createConnection(m_physics->getPvdConnectionManager(), "127.0.0.1", 5425, 100, PxVisualDebuggerExt::getAllConnectionFlags());
}

PhysScene::~PhysScene() {
	if (m_debuggerConnection != nullptr)
		m_debuggerConnection->release();
	m_physics->release();
}

void PhysScene::Update(float dt) {
	float dtClamped = (dt > 1.0f / 30.f) ? 1.0f / 30.0f : dt;
	m_physicsScene->simulate(dtClamped);
	while (m_physicsScene->fetchResults() == false)	{}
}