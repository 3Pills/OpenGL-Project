#include "CollisionShapeHierarchies.h"
#include "Utility.h"

class AIEAllocator : public PxAllocatorCallback {
public:
	virtual ~AIEAllocator(){}

	virtual void* allocate(size_t bytes, const char* type_name, const char* filename, int line)	{
		void* ptr = _aligned_malloc(bytes, 16);
		return ptr;
	}

	virtual void deallocate(void* ptr) {
		_aligned_free(ptr);
	}
};

CollisionShapeHierarchies::CollisionShapeHierarchies(): m_oCamera(50){
	Application::Application();
}
CollisionShapeHierarchies::~CollisionShapeHierarchies(){}

bool CollisionShapeHierarchies::startup(){
	if (!Application::startup()){
		return false;
	}
	glEnable(GL_DEPTH_TEST);

	m_oCamera.setPerspective(glm::radians(50.0f), 1280.0f / 720.0f, 0.1f, 20000.0f);

	setupPhysx();
	PxTransform transform = PxTransform(PxVec3(0, 0, 0), PxQuat((float)PxHalfPi, PxVec3(0, 0, 1)));

	PxRigidStatic* plane = PxCreateStatic(*m_physics, transform, PxPlaneGeometry(), *m_physics_material);

	m_physics_scene->addActor(*plane);
	setupVisualDebugger();

	LoadShader("./data/shaders/vertex.glsl", "", "./data/shaders/fragment.glsl", &m_programID);
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string err = tinyobj::LoadObj(shapes, materials, "./data/models/tank/tank_small.obj");

	if (err.size() > 0){
		printf("%s", err);
		system("pause");
		return false;
	}

	CreateOpenGLBuffers(shapes);

	PxBoxGeometry box = PxBoxGeometry(1, 1, 1);
	PxTransform transformtwo(*(PxMat44*)(&m_oCamera.getWorldTransform()[0]));

	auto dynamicActor = PxCreateDynamic(*m_physics, transformtwo, box, *m_physics_material, PxReal(1));
	dynamicActor->userData = &shapes;

	m_physActors.push_back(dynamicActor);

	Gizmos::create();
	return true;
}

bool CollisionShapeHierarchies::shutdown(){
	m_physics_scene->release();
	m_physics->release();
	m_physics_foundation->release();
	CleanOpenGLBuffers();
	Gizmos::destroy();
	glDeleteProgram(m_programID);
	return Application::shutdown();
}

void CollisionShapeHierarchies::setupVisualDebugger() {
	if (m_physics->getPvdConnectionManager() == NULL)
		return;
	PxVisualDebuggerConnectionFlags connectionFlags = PxVisualDebuggerExt::getAllConnectionFlags();
	auto connection = PxVisualDebuggerExt::createConnection(m_physics->getPvdConnectionManager(), "127.0.0.1", 5425, 100, connectionFlags);
}

void CollisionShapeHierarchies::setupPhysx(){
	m_default_filter_shader = PxDefaultSimulationFilterShader;
	PxAllocatorCallback* my_callback = new AIEAllocator();

	m_physics_foundation = PxCreateFoundation(PX_PHYSICS_VERSION, *my_callback, m_default_error_callback);


	m_physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_physics_foundation, PxTolerancesScale());
	PxInitExtensions(*m_physics);

	PxSceneDesc scene_desc(m_physics->getTolerancesScale());
	scene_desc.gravity = PxVec3(0, -9.807f, 0);
	scene_desc.filterShader = &PxDefaultSimulationFilterShader;
	scene_desc.cpuDispatcher = PxDefaultCpuDispatcherCreate(8);
	m_physics_scene = m_physics->createScene(scene_desc);

	m_physics_material = m_physics->createMaterial(0.5f, 0.5f, 0.5f);
	m_box_material[0] = m_physics->createMaterial(0.5f, 0.5f, 1.3f);
	m_box_material[1] = m_physics->createMaterial(0.5f, 0.5f, 0.0f);
}

bool CollisionShapeHierarchies::update(){
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

	m_physics_scene->simulate(m_fCurrTime > 0.033f ? 0.033f : m_fCurrTime);
	while (m_physics_scene->fetchResults() == false);

	if (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_1)) {
		vec3 pos(m_oCamera.getWorldTransform()[3]), dir(m_oCamera.getWorldTransform()[2]);
		PxVec3 vel(dir.x, dir.y, dir.z);
		vel *= -100;

		vec3 offset = pos + dir * -3.0f;
		PxVec3 pos2(offset.x, offset.y, offset.z);
		PxRigidDynamic* ball = PxCreateDynamic(*m_physics, PxTransform(pos2), PxSphereGeometry(0.5), *m_physics_material, 100000000);
		m_physActors.push_back(ball);
		ball->setLinearVelocity(vel, true);
		m_physics_scene->addActor(*ball);
	}

	for (unsigned int i = 0; i < m_physActors.size(); i++) {
		PxTransform transform = m_physActors[i]->getGlobalPose();
		//get its position
		vec3 position(transform.p.x, transform.p.y, transform.p.z);
		//get its rotation
		glm::quat rotation(transform.q.w, transform.q.x, transform.q.y, transform.q.z);
		//add it as a gizmos
		glm::mat4 rot(rotation);
		if (m_physActors[i]->userData == nullptr) {
			Gizmos::addSphere(position, 0.5, 8, 8, vec4(1, 0, 0, 1), &rot);
		}
		else {
			Gizmos::addAABBFilled(position, vec3(4), vec4(1, 0, 0, 1), &rot);

			PxMat44 m = m_physActors[i]->getGlobalPose();
			//MeshComponent* meshComponent =
		}
	}

	return true;
}

void CollisionShapeHierarchies::draw(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(m_programID);

	int view_proj_uniform = glGetUniformLocation(m_programID, "projView");
	if (view_proj_uniform > -1) 
		glUniformMatrix4fv(view_proj_uniform, 1, GL_FALSE, (float*)&m_oCamera.getProjectionView());

	for (unsigned int i = 0; i < m_meshes.size(); ++i){
		glBindVertexArray(m_meshes[i].m_VAO);
		glDrawElements(GL_TRIANGLES, m_meshes[i].m_indexCount, GL_UNSIGNED_INT, 0);
	}

	Gizmos::addAABBFilled(vec3(0,-0.02,0), vec3(1000,0.01,1000), vec4(1));
	Gizmos::draw(m_oCamera.getProjectionView());
	Application::draw();
}



void CollisionShapeHierarchies::CreateOpenGLBuffers(std::vector<tinyobj::shape_t>& shapes){
	m_meshes.resize(shapes.size());

	for (unsigned int shape_index = 0; shape_index < shapes.size(); ++shape_index){
		std::vector<float> vertex_data;

		unsigned int floatCount = shapes[shape_index].mesh.positions.size();
		floatCount += shapes[shape_index].mesh.normals.size();

		vertex_data.reserve(floatCount);
		vertex_data.insert(vertex_data.end(), shapes[shape_index].mesh.positions.begin(), shapes[shape_index].mesh.positions.end());
		vertex_data.insert(vertex_data.end(), shapes[shape_index].mesh.normals.begin(), shapes[shape_index].mesh.normals.end());

		m_meshes[shape_index].m_indexCount = shapes[shape_index].mesh.indices.size();

		glGenVertexArrays(1, &m_meshes[shape_index].m_VAO);
		glGenBuffers(1, &m_meshes[shape_index].m_VBO);
		glGenBuffers(1, &m_meshes[shape_index].m_IBO);

		glBindVertexArray(m_meshes[shape_index].m_VAO);
		glBindBuffer(GL_ARRAY_BUFFER, m_meshes[shape_index].m_VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float)* floatCount, vertex_data.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_meshes[shape_index].m_IBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, shapes[shape_index].mesh.indices.size() * sizeof(unsigned int), shapes[shape_index].mesh.indices.data(), GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);//position 
		glEnableVertexAttribArray(1);//normal data

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, 0, (void*)(sizeof(float)* shapes[shape_index].mesh.positions.size()));

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
}

void CollisionShapeHierarchies::CleanOpenGLBuffers(){
	glDeleteProgram(m_programID);
	for (unsigned int i = 0; i < m_meshes.size(); ++i){
		glDeleteVertexArrays(1, &m_meshes[i].m_VAO);
		glDeleteBuffers(1, &m_meshes[i].m_VBO);
		glDeleteBuffers(1, &m_meshes[i].m_IBO);
	}
}