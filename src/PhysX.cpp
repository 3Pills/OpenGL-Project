#include "PhysX.h"

#define Assert(val) if (val){}else{ *((char*)0) = 0;}
#define ArrayCount(val) (sizeof(val)/sizeof(val[0]))

class MyAllocator : public physx::PxAllocatorCallback
{
	virtual void* allocate(size_t size, const char* typeName, const char* filename, int line)
	{
		void* new_mem = malloc(size + 32);
		void* result = (char*)new_mem + (16 - ((size_t)new_mem % 16));
		Assert(((size_t)result % 16) == 0);
		*(void**)result = new_mem;
		return (char*)result + 16;
	}

	virtual void deallocate(void* ptr)
	{
		if (ptr)
		{
			void* real_ptr = *(void**)((char*)ptr - 16);
			free(real_ptr);
		}
	}
};

PhysX::PhysX() : m_oCamera(50){
	Application::Application();
}
PhysX::~PhysX(){}

bool PhysX::startup(){
	if (!Application::startup()){
		return false;
	}

	m_oCamera.setPerspective(glm::radians(50.0f), 1280.0f / 720.0f, 0.1f, 20000.0f);
	setupPhysX();


	physx::PxTransform pose = physx::PxTransform(physx::PxVec3(0.0f, 0, 0.0f), physx::PxQuat(physx::PxHalfPi * 1.0f, physx::PxVec3(0.0f, 0.0f, 1.0f)));
	physx::PxRigidStatic* plane = physx::PxCreateStatic(*g_Physics, pose, physx::PxPlaneGeometry(), *g_PhysicsMaterial);

	g_PhysicsScene->addActor(*plane);

	//add a box
	float density = 10;

	for (int i = 0; i < ArrayCount(dynamicActors); ++i)	{
		physx::PxBoxGeometry box(2, 2, 2);
		physx::PxTransform transform(physx::PxVec3(rand() % 10, 10 + (rand() % 15), rand() % 10));
		dynamicActors[i] = PxCreateDynamic(*g_Physics, transform, box, *g_PhysicsMaterial, density);
		//add it to the physX scene
		g_PhysicsScene->addActor(*dynamicActors[i]);
	}

	setupVisualDebugger();
	Gizmos::create();
	return true;
}

bool PhysX::shutdown(){
	Gizmos::destroy();
	return Application::shutdown();
}

bool PhysX::update(){
	if (!Application::update()){
		return false;
	}
	m_oCamera.update(m_fDeltaTime);
	Gizmos::clear();
	Gizmos::addTransform(mat4(1), 10);

	vec4 white(1);
	vec4 black(0, 0, 0, 1);

	for (int i = 0; i <= 20; ++i){
		Gizmos::addLine(vec3(-10 + i, 0, -10), i != 10 ? vec3(-10 + i, 0, 10) : vec3(-10 + i, 0, 0), i != 10 ? black : white);
		Gizmos::addLine(vec3(-10, 0, -10 + i), i != 10 ? vec3(10, 0, -10 + i) : vec3(0, 0, -10 + i), i != 10 ? black : white);
	}

	float dt = (m_fDeltaTime > 1.0f / 30.f) ? 1.0f / 30.0f : m_fDeltaTime;

	g_PhysicsScene->simulate(dt);
	while (g_PhysicsScene->fetchResults() == false)	{}


	for (int i = 0; i < ArrayCount(dynamicActors); ++i)	{
		if (dynamicActors[i]) {
			physx::PxTransform box_transform = dynamicActors[i]->getGlobalPose();
			glm::vec3 pos(box_transform.p.x, box_transform.p.y, box_transform.p.z);
			glm::quat q;

			q.x = box_transform.q.x;
			q.y = box_transform.q.y;
			q.z = box_transform.q.z;
			q.w = box_transform.q.w;

			glm::mat4 rot = glm::mat4(q);

			glm::mat4 model_matrix;
			model_matrix = rot * glm::translate(model_matrix, pos);

			Gizmos::addAABBFilled(pos, vec3(2, 2, 2), vec4(1, 1, 1, 1), &rot);
		}
	}

	return true;
}

void PhysX::draw(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Gizmos::draw(m_oCamera.getProjectionView());
	Application::draw();
}


void PhysX::setupPhysX(){
	physx::PxAllocatorCallback *myCallback = new MyAllocator();

	g_PhysicsFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, *myCallback, gDefaultErrorCallback);
	g_Physics = PxCreatePhysics(PX_PHYSICS_VERSION, *g_PhysicsFoundation, physx::PxTolerancesScale());
	PxInitExtensions(*g_Physics);
	//create physics material  
	g_PhysicsMaterial = g_Physics->createMaterial(0.9f, 0.9f, .1f);
	physx::PxSceneDesc sceneDesc(g_Physics->getTolerancesScale());
	sceneDesc.gravity = physx::PxVec3(0, -10.0f, 0);
	sceneDesc.filterShader = &physx::PxDefaultSimulationFilterShader;
	sceneDesc.cpuDispatcher = physx::PxDefaultCpuDispatcherCreate(1);
	g_PhysicsScene = g_Physics->createScene(sceneDesc);
}

void PhysX::setupVisualDebugger()
{
	if (g_Physics->getPvdConnectionManager() == NULL)
		return;
	
	auto connection = physx::PxVisualDebuggerExt::createConnection(g_Physics->getPvdConnectionManager(), "127.0.0.1", 5425, 100, physx::PxVisualDebuggerExt::getAllConnectionFlags());
}