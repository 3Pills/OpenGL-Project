#include "PhysX.h"

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
	setupMotor();

	physx::PxTransform pose = physx::PxTransform(physx::PxVec3(0.0f, 0, 0.0f), physx::PxQuat(physx::PxHalfPi * 1.0f, physx::PxVec3(0.0f, 0.0f, 1.0f)));
	physx::PxRigidStatic* plane = physx::PxCreateStatic(*m_physics, pose, physx::PxPlaneGeometry(), *m_physicsMaterial);

	m_physicsScene->addActor(*plane);

	//add a box
	float density = 10;

	for (int i = 0; i < 32; ++i)	{
		physx::PxBoxGeometry box(1, 1, 1);
		physx::PxTransform transform(physx::PxVec3(0, 4 + ((float)i * 2.4f), 0));
		m_dynamicActors.push_back(PxCreateDynamic(*m_physics, transform, box, *m_physicsMaterial, density));
		m_extents.push_back(vec3(1));
		//add it to the physX scene
		m_physicsScene->addActor(*m_dynamicActors.back());
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

	m_physicsScene->simulate(dt);
	while (m_physicsScene->fetchResults() == false)	{}


	for (int i = 0; i < m_dynamicActors.size(); ++i)	{
		if (m_dynamicActors[i]) {
			PxTransform box_transform = m_dynamicActors[i]->getGlobalPose();
			vec3 pos(box_transform.p.x, box_transform.p.y, box_transform.p.z);
			quat q;

			q.x = box_transform.q.x;
			q.y = box_transform.q.y;
			q.z = box_transform.q.z;
			q.w = box_transform.q.w;

			mat4 rot = mat4(q);

			mat4 model_matrix;
			model_matrix = rot * glm::translate(model_matrix, pos);

			Gizmos::addAABBFilled(pos, m_extents[i], vec4(1, 0, 0, 1), &rot);
		}
	}

	return true;
}

void PhysX::draw(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Gizmos::draw(m_oCamera.getProjectionView());
	Application::draw();
}

void PhysX::setupMotor(){
	PxMaterial* boxMaterial = m_physics->createMaterial(1, 1, 1);
	float boxHalfSize = 1;
	float gap = .01f;
	float density = 1;
	//create two boxes
	PxBoxGeometry box(boxHalfSize, boxHalfSize, boxHalfSize);
	PxTransform box1Transform(PxVec3(-((boxHalfSize + gap) * 2), 2, 0));
	PxTransform box2Transform(PxVec3(0, 2, 0));

	//Create a PhysX static actor with a box as a collider
	PxRigidStatic* staticBox = PxCreateStatic(*m_physics, box1Transform, box, *boxMaterial);
	m_physicsScene->addActor(*staticBox);
	m_dynamicActors.push_back(staticBox);
	m_extents.push_back(vec3(1));

	// Create a PhysX dynamic actor with a box as a collider
	PxRigidDynamic* dynamicBox = PxCreateDynamic(*m_physics, box2Transform, box, *boxMaterial, density);
	m_physicsScene->addActor(*dynamicBox);
	m_dynamicActors.push_back(dynamicBox);
	m_extents.push_back(vec3(1));

	//set up the constraint frames for the joint so boxes are correctly posiitoned
	PxTransform constraintFrame1 = PxTransform(PxVec3(boxHalfSize + gap, 0, 0));
	PxTransform constraintFrame2 = PxTransform(PxVec3(-(boxHalfSize + gap), 0, 0));
	PxRevoluteJoint *joint = NULL;

	//create the revolute (axle) joint
	joint = PxRevoluteJointCreate(*m_physics, staticBox, constraintFrame1, dynamicBox, constraintFrame2);

	if (joint) //if the joint is successfully created then configure it
	{
		joint->setRevoluteJointFlag(PxRevoluteJointFlag::eDRIVE_ENABLED, true);
		joint->setRevoluteJointFlag(PxRevoluteJointFlag::eLIMIT_ENABLED, false);
		joint->setDriveVelocity(10); //give it some drive
	}
}

void PhysX::setupPhysX(){
	physx::PxAllocatorCallback *myCallback = new MyAllocator();

	m_physicsFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, *myCallback, m_defaultErrorCallback);
	m_physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_physicsFoundation, physx::PxTolerancesScale());
	PxInitExtensions(*m_physics);
	//create physics material  
	m_physicsMaterial = m_physics->createMaterial(0.9f, 0.9f, .1f);
	physx::PxSceneDesc sceneDesc(m_physics->getTolerancesScale());
	sceneDesc.gravity = physx::PxVec3(0, -10.0f, 0);
	sceneDesc.filterShader = &physx::PxDefaultSimulationFilterShader;
	sceneDesc.cpuDispatcher = physx::PxDefaultCpuDispatcherCreate(1);
	m_physicsScene = m_physics->createScene(sceneDesc);
}

void PhysX::setupVisualDebugger()
{
	if (m_physics->getPvdConnectionManager() == NULL)
		return;
	
	auto connection = PxVisualDebuggerExt::createConnection(m_physics->getPvdConnectionManager(), "127.0.0.1", 5425, 100, PxVisualDebuggerExt::getAllConnectionFlags());
}