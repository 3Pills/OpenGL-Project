#include "PhysScene.h"
#include "FBXModel.h"
#include <Gizmos.h>
#include "Utility.h"

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

PxSimulationFilterShader PhysScene::m_defaultFilterShader = PxDefaultSimulationFilterShader;

PxDefaultErrorCallback PhysScene::m_defaultErrorCallback = PxDefaultErrorCallback();
PxDefaultAllocator PhysScene::m_defaultAllocatorCallback = PxDefaultAllocator();

PxVisualDebuggerConnection* PhysScene::m_debuggerConnection;

PxFoundation* PhysScene::m_physicsFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, *(new MyAllocator()), PhysScene::m_defaultErrorCallback);
PxPhysics* PhysScene::m_physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_physicsFoundation, PxTolerancesScale());
PxScene* PhysScene::m_physicsScene = nullptr;

PxRigidStatic* PhysScene::m_plane[6] = {};
PxCooking* PhysScene::m_cooking;

void PhysScene::Init() {
	PxInitExtensions(*m_physics);

	PxSceneDesc sceneDesc(m_physics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0, -40.0f, 0);
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

	//Create a plane at the origin to catch stray physics objects.
	PxTransform planePose = PxTransform(PxVec3(0.f, 0.f, 0.f), PxQuat(PxHalfPi, PxVec3(0.0f, 0.0f, 1.0f)));
	m_plane[0] = PxCreateStatic(*m_physics, planePose, PxPlaneGeometry(), *m_physics->createMaterial(1.f, 1.f, 1.f));
	m_physicsScene->addActor(*m_plane[0]);

	if (m_physics->getPvdConnectionManager() != NULL)
		m_debuggerConnection = PxVisualDebuggerExt::createConnection(m_physics->getPvdConnectionManager(), "127.0.0.1", 5425, 100, PxVisualDebuggerExt::getAllConnectionFlags());
}

void PhysScene::Shutdown() {
	if (m_debuggerConnection != nullptr)
		m_debuggerConnection->release();
	m_physics->release();
}

void PhysScene::Update(const float dt, const bool a_renderGizmos) {
	//Update physics
	m_physicsScene->simulate(dt);
	while (m_physicsScene->fetchResults() == false)	{}

	//Update models based on new physics positions
	int numberActors = m_physicsScene->getNbActors(PxActorTypeFlags(PxActorTypeFlag::eRIGID_DYNAMIC));
	PxActor** dynamicActors = new PxActor*[numberActors] {};
	int actorCount = m_physicsScene->getActors(PxActorTypeFlags(PxActorTypeFlag::eRIGID_DYNAMIC), dynamicActors, numberActors);

	for (int i = 0; i < actorCount; i++) {
		PxRigidActor* actor = (PxRigidActor*)dynamicActors[i];
		PxTransform transform = actor->getGlobalPose();

		//Conversion from PhysX data types to GLM.
		vec3 globalPos = vec3(transform.p.x, transform.p.y, transform.p.z);
		quat globalRot = quat(transform.q.w, transform.q.x, transform.q.y, transform.q.z);

		//Update parented FBXModels to the PhysX. They're stored in the userData slot in PhysX actors.
		if (dynamicActors[i]->userData != nullptr) {
			FBXModel* model = ((FBXModel*)actor->userData);
			model->m_pos = globalPos;

			//Don't rotate the model to the parent PhysX if it is marked otherwise
			if (model->m_parentTransform) {
				model->m_rot = globalRot;
			}
		}

		//Render gizmos of the new physics positions
		if (a_renderGizmos) {
			//Loop through each shape in each actor, drawing a gizmo for it.
			int numberShapes = actor->getNbShapes();
			PxShape** shapes = new PxShape*[numberShapes] {};
			int shapeCount = actor->getShapes(shapes, numberShapes);

			for (int i = 0; i < shapeCount; i++) {
				PxTransform shapeTransform = shapes[i]->getLocalPose();
				vec3 localPos = vec3(shapeTransform.p.x, shapeTransform.p.y, shapeTransform.p.z);
				quat localRot = quat(shapeTransform.q.w, shapeTransform.q.x, shapeTransform.q.y, shapeTransform.q.z);

				//Get the matrix of the local position, rotated by the local and global rotation matrices.
				mat4 localMat = glm::toMat4(globalRot) * glm::toMat4(localRot) * glm::translate(localPos);
				localPos = localMat[3].xyz; //Assign it back to the local pos.

				//Draw different gizmos for different physics geometry
				switch (shapes[i]->getGeometryType()){
				case PxGeometryType::eSPHERE:
					Gizmos::addSphere(globalPos + localPos, shapes[i]->getGeometry().sphere().radius, 12, 12, vec4(0, 1, 0, 1), &localMat);
					break;
				case PxGeometryType::eCAPSULE:
					Gizmos::addCapsule(globalPos + localPos, (shapes[i]->getGeometry().capsule().halfHeight + shapes[i]->getGeometry().capsule().radius) * 2, shapes[i]->getGeometry().capsule().radius, 12, 12, vec4(0, 1, 0, 1), &localMat);
					break;
				case PxGeometryType::eBOX:
					PxVec3 extents = shapes[i]->getGeometry().box().halfExtents;
					Gizmos::addAABB(globalPos + localPos, vec3(extents.x, extents.y, extents.z), vec4(0, 1, 0, 1), &localMat);
					break;
				}
			}
			delete[] shapes;
		}
	}
	delete[] dynamicActors;
}

//Adds bounds around the world, excluding the bottom, as there is already one at the origin by default.
void PhysScene::AddWorldBounds(const vec3 a_extents) {
	PxTransform planePose = PxTransform(PxVec3(a_extents.x, a_extents.y / 2, 0.f), PxQuat(PxPi, PxVec3(0.0f, 1.0f, 0.0f)));
	m_plane[1] = PxCreateStatic(*m_physics, planePose, PxPlaneGeometry(), *m_physics->createMaterial(1.f, 1.f, 1.f));
	m_physicsScene->addActor(*m_plane[1]);

	planePose = PxTransform(PxVec3(-a_extents.x, a_extents.y / 2, 0.f), PxQuat(0, PxVec3(0.0f, 1.0f, 0.0f)));
	m_plane[2] = PxCreateStatic(*m_physics, planePose, PxPlaneGeometry(), *m_physics->createMaterial(1.f, 1.f, 1.f));
	m_physicsScene->addActor(*m_plane[2]);

	planePose = PxTransform(PxVec3(0.f, a_extents.y / 2, a_extents.z), PxQuat(PxHalfPi, PxVec3(0.0f, 1.0f, 0.0f)));
	m_plane[3] = PxCreateStatic(*m_physics, planePose, PxPlaneGeometry(), *m_physics->createMaterial(1.f, 1.f, 1.f));
	m_physicsScene->addActor(*m_plane[3]);

	planePose = PxTransform(PxVec3(0.f, a_extents.y / 2, -a_extents.z), PxQuat(-PxHalfPi, PxVec3(0.0f, 1.0f, 0.0f)));
	m_plane[4] = PxCreateStatic(*m_physics, planePose, PxPlaneGeometry(), *m_physics->createMaterial(1.f, 1.f, 1.f));
	m_physicsScene->addActor(*m_plane[4]);

	planePose = PxTransform(PxVec3(0.f, a_extents.y, 0.f), PxQuat(-PxHalfPi, PxVec3(0.0f, 0.0f, 1.0f)));
	m_plane[5] = PxCreateStatic(*m_physics, planePose, PxPlaneGeometry(), *m_physics->createMaterial(1.f, 1.f, 1.f));
	m_physicsScene->addActor(*m_plane[5]);
}

PxRigidStatic* PhysScene::AddRigidBodyStatic(const PxTransform a_transform, PxGeometry* a_geometry, PxMaterial* a_physMaterial, void* a_userData){
	PxRigidStatic* m_phys = PxCreateStatic(*m_physics, a_transform, *a_geometry, *a_physMaterial);

	if (a_userData != nullptr) {
		m_phys->userData = a_userData;
		FBXModel* model = ((FBXModel*)a_userData);

		PxGeometryHolder* geometry = new PxGeometryHolder();
		geometry->storeAny(*a_geometry);

		vec3 modelExtents = vec3(0);
		switch (geometry->getType()) {
		case PxGeometryType::eSPHERE:
			modelExtents = vec3(0, geometry->sphere().radius, 0);
			break;
		case PxGeometryType::eCAPSULE:
			modelExtents = vec3(geometry->capsule().halfHeight, 0, 0);

			//Capsules are sideways by default, need to rotate soulspear to follow that.
			model->m_modTransform = (glm::rotate(glm::radians(90.0f), vec3(0, 0, 1))) * model->m_modTransform;
			break;
		case PxGeometryType::eBOX:
			modelExtents = vec3(0, geometry->box().halfExtents.y, 0);
		}

		//Set model position and rotation to the static rigidbody. This is the only time they are set, as it won't change
		model->m_pos = vec3(a_transform.p.x, a_transform.p.y, a_transform.p.z) - modelExtents;
		model->m_rot = quat(a_transform.q.w, a_transform.q.x, a_transform.q.y, a_transform.q.z);
	}

	m_physicsScene->addActor(*m_phys);
	return m_phys;
}

PxRigidDynamic* PhysScene::AddRigidBodyDynamic(const PxTransform a_transform, PxGeometry* a_geometry, PxMaterial* a_physMaterial, void* a_userData, const float a_density){
	PxRigidDynamic* m_phys = PxCreateDynamic(*m_physics, a_transform, *a_geometry, *a_physMaterial, a_density);

	//Align the PhysX with the FBXModel origin.
	if (a_userData != nullptr) {
		m_phys->userData = a_userData;
		FBXModel* model = ((FBXModel*)a_userData);

		PxGeometryHolder* geometry = new PxGeometryHolder();
		geometry->storeAny(*a_geometry);

		int numberShapes = m_phys->getNbShapes();
		PxShape* shapes;
		m_phys->getShapes(&shapes, numberShapes);

		vec3 modelExtents = vec3(0);
		switch (geometry->getType()) {
		case PxGeometryType::eSPHERE:
			modelExtents = vec3(0, geometry->sphere().radius, 0);
			break;
		case PxGeometryType::eCAPSULE:
			modelExtents = vec3(geometry->capsule().halfHeight, 0, 0);

			//Capsules are sideways by default, need to rotate soulspear to follow that.
			model->m_modTransform = (glm::rotate(glm::radians(90.0f), vec3(0, 0, 1))) * model->m_modTransform;
			break;
		case PxGeometryType::eBOX:
			modelExtents = vec3(0, geometry->box().halfExtents.y, 0);
		}

		PxTransform relativePose = PxTransform((*(PxVec3*)(&modelExtents[0])));
		shapes->setLocalPose(relativePose);

		//Set the centre of mass to the new relative position
		m_phys->setCMassLocalPose(relativePose);

		delete geometry;
	}

	m_physicsScene->addActor(*m_phys);
	return m_phys;
}

PxRigidStatic* PhysScene::AttachRigidBodyTriangle(const PxTransform a_transform, PxMaterial* a_physMaterial, void* a_userData, const float a_physModelScale) {
	if (a_userData == nullptr || m_cooking == nullptr)
		return nullptr;

	PxRigidStatic* m_phys = PxCreateStatic(*m_physics, a_transform, PxBoxGeometry(1, 1, 1), *a_physMaterial);

	m_phys->userData = a_userData;
	int numberVerts = 0;
	int numberIndices = 0;
	FBXModel* model = ((FBXModel*)a_userData);

	//find out how many verts there are in total in model
	for (unsigned int i = 0; i < model->m_file->getMeshCount(); ++i) {
		FBXMeshNode* mesh = model->m_file->getMeshByIndex(i);
		numberVerts += mesh->m_vertices.size();
		numberIndices += mesh->m_indices.size();
	}

	//reserve space for vert buffer
	PxVec3 *verts = new PxVec3[numberVerts];
	PxU32 *indices = new PxU32[numberIndices];
	int vertIDX = 0;
	int indexIDX = 0;

	//copy our verts from all the sub meshes and tranform them into the same space
	for (unsigned int i = 0; i < model->m_file->getMeshCount(); ++i){
		FBXMeshNode* mesh = model->m_file->getMeshByIndex(i);

		int numberV = mesh->m_vertices.size();
		for (int vertCount = 0; vertCount< numberV; vertCount++){
			glm::vec4 temp = mesh->m_globalTransform * glm::scale(vec3(a_physModelScale)) * mesh->m_vertices[vertCount].position;
			verts[vertIDX++] = PxVec3(temp.x, temp.y, temp.z);
		}

		int numberI = mesh->m_indices.size();
		for (int indexCount = 0; indexCount < numberI; indexCount++){
			indices[indexIDX++] = mesh->m_indices[indexCount];
		}
	}

	PxTriangleMeshDesc meshDesc;
	meshDesc.points.count = numberVerts;
	meshDesc.points.stride = sizeof(PxVec3);
	meshDesc.points.data = verts;
	int triCount = numberIndices / 3;
	meshDesc.triangles.count = triCount;
	meshDesc.triangles.stride = 3 * sizeof(PxU32);
	meshDesc.triangles.data = indices;

	PxDefaultMemoryOutputStream* buf = new PxDefaultMemoryOutputStream();
	assert(m_cooking->cookTriangleMesh(meshDesc, *buf));

	PxU8* contents = buf->getData();
	PxU32 size = buf->getSize();
	PxDefaultMemoryInputData input(contents, size);
	PxTriangleMesh* triangleMesh = m_physics->createTriangleMesh(input);
	PxTransform pose = PxTransform(PxVec3(0));
	PxShape* convexShape = m_phys->createShape(PxTriangleMeshGeometry(triangleMesh), *a_physMaterial, pose);

	return m_phys;
}

PxRigidDynamic* PhysScene::AttachRigidBodyConvex(const PxTransform a_transform, PxMaterial* a_physMaterial, void* a_userData, const float a_density, const float a_physModelScale) {
	if (a_userData == nullptr || m_cooking == nullptr)
		return nullptr;

	PxRigidDynamic* m_phys = PxCreateDynamic(*m_physics, a_transform, PxBoxGeometry(1, 1, 1), *a_physMaterial, a_density);

	m_phys->userData = a_userData;
	int numberVerts = 0;
	FBXModel* model = ((FBXModel*)a_userData);

	//find out how many verts there are in total in model
	for (unsigned int i = 0; i < model->m_file->getMeshCount(); ++i) {
		FBXMeshNode* mesh = model->m_file->getMeshByIndex(i);
		numberVerts += mesh->m_vertices.size();
	}

	//reserve space for vert buffer
	PxVec3 *verts = new PxVec3[numberVerts];
	int vertIDX = 0;

	//copy our verts from all the sub meshes and tranform them into the same space
	for (unsigned int i = 0; i < model->m_file->getMeshCount(); ++i){
		FBXMeshNode* mesh = model->m_file->getMeshByIndex(i);
		int numberV = mesh->m_vertices.size();
		for (int vertCount = 0; vertCount < numberV; vertCount++){
			//Scale the global transform by an arbitrary number, based on what the model individually requires.
			glm::vec4 temp = mesh->m_globalTransform * glm::scale(vec3(a_physModelScale)) * mesh->m_vertices[vertCount].position;
			verts[vertIDX++] = PxVec3(temp.x, temp.y, temp.z);
		}
	}

	PxConvexMeshDesc convexDesc;
	convexDesc.points.count = numberVerts;
	convexDesc.points.stride = sizeof(PxVec3);
	convexDesc.points.data = verts;
	convexDesc.flags = PxConvexFlag::eCOMPUTE_CONVEX;
	convexDesc.vertexLimit = 128;

	PxDefaultMemoryOutputStream* buf = new PxDefaultMemoryOutputStream();
	assert(m_cooking->cookConvexMesh(convexDesc, *buf));

	PxU8* contents = buf->getData();
	PxU32 size = buf->getSize();
	PxDefaultMemoryInputData input(contents, size);
	PxConvexMesh* convexMesh = m_physics->createConvexMesh(input);
	PxTransform pose = PxTransform(PxVec3(0.0f));
	PxShape* convexShape = m_phys->createShape(PxConvexMeshGeometry(convexMesh), *a_physMaterial, pose);

	//Set the centre of mass to the centre of the new convexMesh.
	m_phys->setCMassLocalPose(PxTransform(convexMesh->getLocalBounds().getCenter()));

	//remove the placeholder box we started with
	int numberShapes = m_phys->getNbShapes();
	PxShape** shapes = (PxShape**)_aligned_malloc(sizeof(PxShape*)*numberShapes, 16);
	m_phys->getShapes(shapes, numberShapes);
	m_phys->detachShape(**shapes);

	delete(verts); //delete our temporary vert buffer.

	m_physicsScene->addActor(*m_phys);

	return m_phys;
}

PxRigidStatic* PhysScene::AddHeightMap(float* a_heightMap, PxMaterial* a_physMaterial, glm::vec2 a_dims, glm::vec3 a_scale, unsigned int a_downScale) {
	if (a_dims.x == 0 || a_dims.y == 0)
		return nullptr;

	PxHeightFieldDesc hfDesc;
	hfDesc.format = PxHeightFieldFormat::eS16_TM;

	//Pass in dimensions of mesh
	hfDesc.nbRows = (PxU32)(a_dims.x / a_downScale);
	hfDesc.nbColumns = (PxU32)(a_dims.y / a_downScale);
	hfDesc.thickness = -100.0f;

	int sampleCount = (int)(hfDesc.nbRows * hfDesc.nbColumns);

	PxHeightFieldSample* heightSampleData = new PxHeightFieldSample[sampleCount];

	int heightMapResolution = 20000;

	int index = 0;
	for (unsigned int i = 0; i < hfDesc.nbRows; i++) {
		for (unsigned int j = 0; j < hfDesc.nbColumns; j++) {
			heightSampleData[((i + 1) * hfDesc.nbRows) - (j + 1)].height = PxI16(a_heightMap[(index * a_downScale)] * heightMapResolution);
			heightSampleData[((i + 1) * hfDesc.nbRows) - (j + 1)].materialIndex0 = 0;
			heightSampleData[((i + 1) * hfDesc.nbRows) - (j + 1)].materialIndex1 = 0;
			index++;
		}
		index += (hfDesc.nbRows * (a_downScale - 1));
	}

	hfDesc.samples.data = heightSampleData;
	hfDesc.samples.stride = sizeof(PxHeightFieldSample);

	//Conversion from texture size to mesh size.
	float xRatio = (a_scale.x / (hfDesc.nbRows - 1)), yRatio = (a_scale.z / (hfDesc.nbColumns - 1));

	PxHeightField* aHeightField = m_physics->createHeightField(hfDesc);
	PxHeightFieldGeometry hfGeom(aHeightField, PxMeshGeometryFlags(), (1.0f / heightMapResolution) * (a_scale.y), xRatio, yRatio);
	PxTransform pose = PxTransform(PxVec3(((hfDesc.nbRows - 1) * xRatio) / 2, 0, -((hfDesc.nbColumns - 1) * yRatio) / 2));

	quat rot = glm::toQuat(glm::rotate(glm::radians(-90.f), vec3(0, 1, 0)));
	pose.q = PxQuat(rot.x, rot.y, rot.z, rot.w);

	PxRigidStatic* m_phys = PxCreateStatic(*m_physics, pose, hfGeom, *a_physMaterial);

	m_physicsScene->addActor(*m_phys);

	return m_phys;
}

PxCloth* PhysScene::AddCloth(const glm::vec3& a_pos, unsigned int& a_vertexCount, unsigned int& a_indexCount, const glm::vec3* a_vertices, unsigned int* a_indices) {
	// set up the cloth description
	PxClothMeshDesc clothDesc;
	clothDesc.points.count = a_vertexCount;
	clothDesc.invMasses.count = a_vertexCount;
	clothDesc.triangles.count = a_indexCount / 3;


	// set up the particles for each vertex
	PxClothParticle* particles = new PxClothParticle[a_vertexCount];
	for (unsigned int i = 0; i < a_vertexCount; ++i)
	{
		particles[i].pos.x = a_vertices[i].x;
		particles[i].pos.y = a_vertices[i].y;
		particles[i].pos.z = a_vertices[i].z;

		particles[i].invWeight = 1.f;
	}

	clothDesc.points.data = particles;
	clothDesc.points.stride = sizeof(PxClothParticle);
	clothDesc.invMasses.data = &particles->invWeight;
	clothDesc.invMasses.stride = sizeof(PxClothParticle);
	clothDesc.triangles.data = a_indices;
	clothDesc.triangles.stride = sizeof(unsigned int) * 3;

	PxClothFabric* fabric = PxClothFabricCreate(*m_physics, clothDesc, PxVec3(0, -1, 0));

	PxTransform clothTransform(*(PxVec3*)(&a_pos.x));
	PxCloth* cloth = m_physics->createCloth(clothTransform, *fabric, particles, PxClothFlag::eSCENE_COLLISION);
	cloth->setSolverFrequency(1.f);
	m_physicsScene->addActor(*cloth);

	return cloth;
}

PxController* PhysScene::AddPlayerController(const PxExtendedVec3 a_pos, PxMaterial* a_physMaterial, void* a_userData) {
	PlayerHitReport* hitReport = new PlayerHitReport();
	PxControllerManager* characterManager = PxCreateControllerManager(*m_physicsScene);

	PxCapsuleControllerDesc desc;
	desc.height = 9.f;
	desc.radius = 5.f;
	desc.position.set(a_pos.x, a_pos.y, a_pos.z);
	desc.material = a_physMaterial;
	desc.reportCallback = hitReport; //connect it to our collision detection routine
	desc.density = 100.0f;
	desc.stepOffset = 0.1f;

	PxController* playerController = characterManager->createController(desc);
	playerController->setUserData(hitReport); //Attach the hitReport to the controller as userdata
	playerController->getActor()->userData = a_userData; //Attach FBXModel argument to the actor of the playercontroller.

	FBXModel* model = ((FBXModel*)a_userData);
	//Capsules are angled 90 degrees, rotate the attached model, and centre it.
	model->m_modTransform = glm::translate(-vec3(0, (desc.height + desc.radius) / 1.35f, 0)) * model->m_modTransform;

	hitReport->clearPlayerContactNormal(); //initialize the contact normal (what we are in contact with)

	return playerController;
}

void PlayerHitReport::onShapeHit(const PxControllerShapeHit &hit) {
	m_playerContactNormal = hit.worldNormal;

	PxRigidDynamic* actor = hit.actor->isRigidDynamic();
	if (actor ) {
		//actor->addForce(hit.dir * hit.length);
	}
}

void PlayerHitReport::onControllerHit(const PxControllersHit &hit) {}
void PlayerHitReport::onObstacleHit(const PxControllerObstacleHit &hit) {}

ClothData::ClothData(const unsigned int a_particleSize, const unsigned int a_rows, const unsigned int a_columns, const char* a_filename)
	: m_cloth(nullptr), m_rows(a_rows), m_columns(a_columns) {
	//Load the shader for the cloth
	LoadShader("./data/shaders/cloth.vs", 0, "./data/shaders/gbuffer_textured.fs", &m_program);
	// this position will represent the top middle vertex
	glm::vec3 clothPosition = glm::vec3(0, 12, 0);
	// shifting grid position for looks
	float halfWidth = a_rows * a_particleSize * 0.5f;
	// generate vertices for a grid with texture coordinates
	m_vertexCount = a_rows * a_columns;

	//Initialise arrays for array buffer data.
	m_vertices = new vec3[m_vertexCount] {};
	m_texCoords = new vec2[m_vertexCount] {};
	m_normals = new vec3[m_vertexCount] {};

	for (unsigned int r = 0; r < a_rows; ++r) {
		for (unsigned int c = 0; c < a_columns; ++c) {
			m_vertices[r * a_columns + c].x = clothPosition.x + a_particleSize * c;
			m_vertices[r * a_columns + c].y = clothPosition.y;
			m_vertices[r * a_columns + c].z = clothPosition.z + a_particleSize * r - halfWidth;
			m_texCoords[r * a_columns + c].x = 1.0f - r / (a_rows - 1.0f);
			m_texCoords[r * a_columns + c].y = 1.0f - c / (a_columns - 1.0f);
		}
	}

	m_indexCount = (a_rows - 1) * (a_columns - 1) * 2 * 3;
	m_indices = new unsigned int[m_indexCount];
	unsigned int* index = m_indices;
	for (unsigned int r = 0; r < (a_rows - 1); ++r) {
		for (unsigned int c = 0; c < (a_columns - 1); ++c) {
			// indices for the 4 quad corner vertices
			unsigned int i0 = r * a_columns + c;
			unsigned int i1 = i0 + 1;
			unsigned int i2 = i0 + a_columns;
			unsigned int i3 = i2 + 1;
			// every second quad changes the triangle order
			if ((c + r) % 2) {
				*index++ = i0; *index++ = i2; *index++ = i1;
				*index++ = i1; *index++ = i2; *index++ = i3;
			}
			else {
				*index++ = i0; *index++ = i2; *index++ = i3;
				*index++ = i0; *index++ = i3; *index++ = i1;
			}
		}
	}
}

ClothData::~ClothData() {
	delete[] m_vertices;
	delete[] m_indices;
	delete[] m_texCoords;

	glDeleteTextures(1, &m_texture);
	glDeleteVertexArrays(1, &m_VAO);
	glDeleteBuffers(1, &m_VBO);
	glDeleteBuffers(1, &m_IBO);
}

void ClothData::GenerateGLBuffers() {
	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO);
	glGenBuffers(1, &m_IBO);

	glBindVertexArray(m_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO);

	//Cloth objects will hold Positional, TexCoord and Normal data, respectively.
	glBufferData(GL_ARRAY_BUFFER, (sizeof(vec3) + sizeof(vec2) + sizeof(vec3)) * m_vertexCount, 0, GL_STATIC_DRAW);
	//Assigning each array into the array buffer.
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec3) * m_vertexCount, m_vertices);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vec3)* m_vertexCount, sizeof(vec2)* m_vertexCount, m_texCoords);
	glBufferSubData(GL_ARRAY_BUFFER, (sizeof(vec3) + sizeof(vec2))* m_vertexCount, sizeof(vec3)* m_vertexCount, m_normals);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)* m_indexCount, m_indices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(vec3)* m_vertexCount));
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, (void*)((sizeof(vec3) + sizeof(vec2))* m_vertexCount));

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void ClothData::Render(Camera* a_camera) {
	if (m_cloth == nullptr) return;

	glUseProgram(m_program);
	int loc = glGetUniformLocation(m_program, "view");
	glUniformMatrix4fv(loc, 1, GL_FALSE, (float*)&a_camera->GetView());

	loc = glGetUniformLocation(m_program, "projView");
	glUniformMatrix4fv(loc, 1, GL_FALSE, (float*)&a_camera->GetProjectionView());

	loc = glGetUniformLocation(m_program, "textureScale");
	glUniform1f(loc, 1.0f);

	//Updating the position and normal data of the cloth. To access this data the cloth must be locked.
	PxClothParticleData* data = m_cloth->lockParticleData(PxDataAccessFlag::eREADABLE);
	unsigned int particleCount = m_cloth->getNbParticles();
	//Loop through and update the positions with simple data reading.
	for (unsigned int i = 0; i < particleCount; i++) {
		m_vertices[i] = vec3(data->particles[i].pos.x, data->particles[i].pos.y, data->particles[i].pos.z) + vec3(0, 40, 0);
	}
	//Loop through the vertex data and generate normals off it
	for (unsigned int i = 0; i < m_vertexCount; i++){
		//Calculate the adjacent vertex to sample, sampling the previous vertex 
		//unless at the start of the array, cause 0 is easier to type than vertexCount.
		unsigned int x = (i < m_rows) ? i + m_rows : i - m_rows;
		unsigned int y = (i % m_columns == 0) ? i + 1 : i - 1;

		//Use cross product to calculate the normal of the current vertex. Assign it to the normals array.
		m_normals[i] = glm::normalize(glm::cross(m_vertices[x] - m_vertices[i], m_vertices[y] - m_vertices[i]));
	}
	//Open the buffer for writing and update the arrays.
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3) * m_vertexCount, m_vertices);
	//Normals are stored third, so we have to skip the texcoords with the correct buffer space
	glBufferSubData(GL_ARRAY_BUFFER, (sizeof(vec3) + sizeof(vec2))* m_vertexCount, sizeof(vec3)* m_vertexCount, m_normals);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//Draw the cloth
	glBindVertexArray(m_VAO);
	glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, 0);

	data->unlock();
}