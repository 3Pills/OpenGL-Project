#include "VirtualWorld.h"
#include "AntTweakBar.h"
#include <string>
#include "stb_image.h"
#include <cmath>

//VirtualWorld app stored as a global variable. Mainly for AntTweakBar to access the application during button callbacks.
VirtualWorld* m_app;

VirtualWorld::VirtualWorld() : m_oCamera(50), m_pHeightScale(50.f), m_pOct(6), m_pAmp(0.9f), m_pPers(0.2f), m_ambCol(vec3(0.35f)),
m_pPos(vec3(0)), m_pScale(vec3(1)), m_pTextureScale(0.2f), m_pFresnelScale(0.5f), m_pRoughness(0.75f)
{
	Application::Application();
	m_app = this;
}
VirtualWorld::~VirtualWorld(){}

bool VirtualWorld::startup(){
	if (!Application::startup()){
		return false;
	}
	Gizmos::create();

	//AntTweakBar Initialisation.
	TwInit(TW_OPENGL_CORE, nullptr);
	TwWindowSize(m_iWidth, m_iHeight);

	//Binding input callbacks for ATB GUI.
	glfwSetMouseButtonCallback(m_window, OnMouseButton);
	glfwSetCursorPosCallback(m_window, OnMousePosition);
	glfwSetScrollCallback(m_window, OnMouseScroll);
	glfwSetKeyCallback(m_window, OnKey);
	glfwSetCharCallback(m_window, OnChar);
	glfwSetWindowSizeCallback(m_window, [](GLFWwindow*, int width, int height){
		m_app->resize(width, height); 
	});

	//Initialise camera
	m_oCamera.SetPerspective(glm::radians(50.0f), 1280.0f / 720.0f, 0.1f, 20000.0f);

	//Initialise PhysX
	PhysScene::Init();

	//Shader Initialisation
	//Opaque Geometry Shaders
	LoadShader("./data/shaders/gbuffer.vs", 0, "./data/shaders/gbuffer.fs", &m_gBufferProgram);
	LoadShader("./data/shaders/perlin.vs", 0, "./data/shaders/gbuffer_textured.fs", &m_proceduralProgram);

	//Light Pre-Pass Shaders
	LoadShader("./data/shaders/directional_light.vs", 0, "./data/shaders/directional_light.fs", &m_dirLightProgram);
	LoadShader("./data/shaders/point_light.vs", 0, "./data/shaders/point_light.fs", &m_pointLightProgram);

	//Final Render Shader
	LoadShader("./data/shaders/composite.vs", 0, "./data/shaders/composite.fs", &m_compositeProgram);

	//Add some models.
	BuildQuad();
	BuildCube();
	BuildFrameBuffers();


	//AntTweakBar GUI Settings Initialisation
	TwBar* m_generalBar = TwNewBar("Debugging");
	TwAddVarRW(m_generalBar, "Lock Camera", TW_TYPE_BOOL8, &m_oCamera.m_locked, "");
	TwAddVarRW(m_generalBar, "No Z-Buffer", TW_TYPE_BOOL8, &m_debug[0], "group=Gizmos");
	TwAddVarRW(m_generalBar, "Draw Grid", TW_TYPE_BOOL8, &m_debug[1], "group=Gizmos");
	TwAddVarRW(m_generalBar, "Draw PhysX", TW_TYPE_BOOL8, &m_debug[2], "group=Gizmos");
	TwAddVarRW(m_generalBar, "Draw Particles", TW_TYPE_BOOL8, &m_debug[3], "group=Gizmos");
	TwAddVarRW(m_generalBar, "Draw Point Lights", TW_TYPE_BOOL8, &m_debug[4], "group=Gizmos");
	TwAddVarRW(m_generalBar, "Draw Directional Lights", TW_TYPE_BOOL8, &m_debug[5], "group=Gizmos");
	TwAddVarRW(m_generalBar, "Draw AI Nodes", TW_TYPE_BOOL8, &m_debug[6], "group=Gizmos");

	//AI node position initialisation
	m_nodes[0].m_pos = vec3(-200, 27, -200);
	m_nodes[1].m_pos = vec3(-130, 35, -213);
	m_nodes[2].m_pos = vec3(0, 50, -235);
	m_nodes[3].m_pos = vec3(150, 20, -205);
	m_nodes[4].m_pos = vec3(200, 50, 15);
	m_nodes[5].m_pos = vec3(170, 45, 180);
	m_nodes[6].m_pos = vec3(30, 53, 200);
	m_nodes[7].m_pos = vec3(-110, 50, 230);
	m_nodes[8].m_pos = vec3(-140, 55, 110);
	m_nodes[9].m_pos = vec3(-190, 45, -75);

	//Link nodes between each other
	for (unsigned int i = 0; i < 10; i++) {
		m_nodes[i].m_next = (i != 9) ? &m_nodes[i + 1] : &m_nodes[0];
		m_nodes[i].m_prev = (i != 0) ? &m_nodes[i - 1] : &m_nodes[9];
	}

	TwBar* m_modelsBar = TwNewBar("Models");
	AddFBXModel(new FBXModel("./data/models/characters/Pyro/pyro.fbx", 0.3f, 2.0f, glm::scale(vec3(0.01f))));
	m_FBXModels.back()->m_parentTransform = false; //Disable player model rotating with PlayerCollider (It doesn't rotate)
	m_player = new Player(vec3(0.0f, 55.0f, 0.0f), 2.0f, m_FBXModels.back());
	m_navi = new Navi(m_player->GetController()->getActor());

	//Add a bunch of AI entities
	//Create the transform override for the skeleton models.
	mat4 spookyTransform = glm::translate(vec3(0.1f, 0.4f, 0)) * glm::scale(vec3(0.1f)) * glm::rotate(glm::radians(-90.0f), vec3(1, 0, 0));

	AddFBXModel(new FBXModel("./data/models/characters/Spooky/skeleton.fbx", 0.3f, 2.0f, spookyTransform));
	m_FBXModels.back()->m_parentTransform = false;
	m_AIEntities.push_back(new AIEntity(vec3(-130, 85, -213), 2.0f, &m_nodes[1], m_FBXModels.back()));

	AddFBXModel(new FBXModel("./data/models/characters/Spooky/skeleton.fbx", 0.3f, 2.0f, spookyTransform));
	m_FBXModels.back()->m_parentTransform = false;
	m_AIEntities.push_back(new AIEntity(vec3(30, 83, 200), 2.0f, &m_nodes[6], m_FBXModels.back()));

	AddFBXModel(new FBXModel("./data/models/characters/Spooky/skeleton.fbx", 0.3f, 2.0f, spookyTransform));
	m_FBXModels.back()->m_parentTransform = false;
	m_AIEntities.push_back(new AIEntity(vec3(-190, 85, -75), 2.0f, &m_nodes[9], m_FBXModels.back()));

	//Add a static tank to the scene.
	AddFBXModel(new FBXModel("./data/models/tank/battle_tank.fbx", 0.3f, 2.0f, glm::scale(vec3(0.3f))));
	PhysScene::AddRigidBodyStatic(PxTransform(PxVec3(-80, 37, -40), PxQuat(PxReal(glm::radians(25.0f)), PxVec3(1,0,0))), &PxBoxGeometry(35, 16, 55), PhysScene::m_physics->createMaterial(0.0f, 0.0f, 0.1f), m_FBXModels.back());

	//Add some physics-enabled bombs
	mat4 bombTransform = glm::translate(vec3(0.1f, 0.4f, 0)) * glm::scale(vec3(0.1875f)) * glm::rotate(glm::radians(-90.0f), vec3(1, 0, 0));
	AddFBXModel(new FBXModel("./data/models/items/bomb.fbx", 0.3f, 2.0f, bombTransform));
	PhysScene::AddRigidBodyDynamic(PxTransform(-20, 60, -20), &PxSphereGeometry(3.0f), PhysScene::m_physics->createMaterial(0.0f, 0.0f, 0.1f), m_FBXModels.back(), 10.f);

	AddFBXModel(new FBXModel("./data/models/items/bomb.fbx", 0.3f, 2.0f, bombTransform));
	PhysScene::AddRigidBodyDynamic(PxTransform(-20, 60, -20), &PxSphereGeometry(3.0f), PhysScene::m_physics->createMaterial(0.0f, 0.0f, 0.1f), m_FBXModels.back(), 10.f);

	AddFBXModel(new FBXModel("./data/models/items/bomb.fbx", 0.3f, 2.0f, bombTransform));
	PhysScene::AddRigidBodyDynamic(PxTransform(-20, 60, -20), &PxSphereGeometry(3.0f), PhysScene::m_physics->createMaterial(0.0f, 0.0f, 0.1f), m_FBXModels.back(), 10.f);

	AddFBXModel(new FBXModel("./data/models/items/bomb.fbx", 0.3f, 2.0f, bombTransform));
	PhysScene::AddRigidBodyDynamic(PxTransform(-20, 60, -20), &PxSphereGeometry(3.0f), PhysScene::m_physics->createMaterial(0.0f, 0.0f, 0.1f), m_FBXModels.back(), 10.f);

	
	//Create Particle GUI bar and add particle emitters.
	TwBar* m_particlesBar = TwNewBar("Particles");
	AddParticleEmitter(new GPUEmitter(vec3(0), vec3(1), 35, 0.5f, 1.5f, 1.0f, 2.0f, 0.0f, 0.75f, 1.0f, 0.75f, vec4(1), vec4(1), EMIT_POINT, PMOVE_LINEAR, "./data/textures/particles/glow.png"));
	AddParticleEmitter(new GPUEmitter(vec3(0, 10, 0), vec3(80), 120, 1.5f, 3.5f, 1.0f, 2.0f, 1.0f, 0.75f, 1.0f, 1.0f, vec4(1, 1, 0.65f, 1), vec4(0.65, 0.65, 0, 1), EMIT_RECTANGLE, PMOVE_WAVE, "./data/textures/particles/glow.png"));
	AddParticleEmitter(new GPUEmitter(vec3(0), vec3(1), 50, 1.0f, 2.0f, 1.0f, 2.0f, 1.0f, 0.5f, 1.0f, 0.5f, vec4(1, 0.5, 0.5, 1), vec4(1, 0, 0, 1), EMIT_POINT, PMOVE_LINEAR, "./data/textures/particles/glow.png"));

	TwBar* m_lightingBar = TwNewBar("Lighting"); //Lighting window. Allows modification of lighting data.

	//Buttons to add and remove point lights from the program
	//These buttons use lambda callbacks, accessing the globally available m_app to modify class variables.
	TwAddButton(m_lightingBar, "Add Point Light", [](void*){m_app->AddPointLight(); }, nullptr, "");
	TwAddButton(m_lightingBar, "Remove Point Light", [](void*){
		TwBar* m_lightingBar = TwGetBarByName("Lighting");
		if (m_lightingBar == nullptr || m_app->m_pointLights.size() <= 0) return;

		std::string prefix("PL" + std::to_string(m_app->m_pointLights.size()) + "_");

		TwRemoveVar(m_lightingBar, std::string(prefix + "X").c_str());
		TwRemoveVar(m_lightingBar, std::string(prefix + "Y").c_str());
		TwRemoveVar(m_lightingBar, std::string(prefix + "Z").c_str());
		TwRemoveVar(m_lightingBar, std::string(prefix + "Color").c_str());
		TwRemoveVar(m_lightingBar, std::string(prefix + "Radius").c_str());

		m_app->m_pointLights.pop_back();
	}, nullptr, "");

	//Buttons to add and remove directional lights from the program
	TwAddButton(m_lightingBar, "Add Directional Light", [](void*){ m_app->AddDirectionalLight(); }, nullptr, "");
	TwAddButton(m_lightingBar, "Remove Directional Light", [](void*){
		TwBar* m_lightingBar = TwGetBarByName("Lighting");
		if (m_lightingBar == nullptr || m_app->m_dirLights.size() <= 0) return;

		std::string prefix("DL" + std::to_string(m_app->m_dirLights.size()) + "_");

		TwRemoveVar(m_lightingBar, std::string(prefix + "Direction").c_str());
		TwRemoveVar(m_lightingBar, std::string(prefix + "Color").c_str());

		m_app->m_dirLights.pop_back();
	}, nullptr, "");

	TwAddVarRW(m_lightingBar, "Ambient Color", TW_TYPE_COLOR3F, &m_ambCol, "");
	AddPointLight(vec3(-70.5f, 55.5f, -33.5f), vec3(1,0,0), 25);
	AddDirectionalLight(vec3(-1), vec3(1)); 
	
	//Initialise Procedural Landscape
	vec2 meshDims = glm::vec2(512, 512);
	glm::ivec2 textDims = glm::ivec2(256,256);

	BuildProceduralGrid(meshDims, textDims);
	BuildPerlinTexture(textDims, m_pOct, m_pAmp, m_pPers);
	PhysScene::AddHeightMap(m_pData, PhysScene::m_physics->createMaterial(1, 1, 1), textDims, vec3(meshDims.x, m_pHeightScale + 2.f, meshDims.y), 4);

	//Load Perlin Diffuse texture
	LoadTexture("./data/textures/grass.tga", &m_pTexture);

	//Add a wall around the world, based on the size of the perlin mesh.
	PhysScene::AddWorldBounds(vec3(meshDims.x / 2, 1000, meshDims.y / 2));

	//Add procedural generation variables to debugging GUI window. 
	TwAddVarRW(m_generalBar, "Roughness",		TW_TYPE_FLOAT, &m_pRoughness,	"group=ProceduralGeneration min=0 step=0.01");
	TwAddVarRW(m_generalBar, "Fresnel Scale",	TW_TYPE_FLOAT, &m_pFresnelScale,"group=ProceduralGeneration min=0 step=0.01");
	TwAddVarRW(m_generalBar, "Scale",			TW_TYPE_FLOAT, &m_pHeightScale, "group=ProceduralGeneration min=0 step=0.05");
	TwAddVarRW(m_generalBar, "Octaves",			TW_TYPE_INT32, &m_pOct,			"group=ProceduralGeneration min=1 max=10"); //Don't want octaves to go too high. Will choke CPU if reloaded.
	TwAddVarRW(m_generalBar, "Amplitude",		TW_TYPE_FLOAT, &m_pAmp,			"group=ProceduralGeneration min=0 step=0.01");
	TwAddVarRW(m_generalBar, "Persistance",		TW_TYPE_FLOAT, &m_pPers,		"group=ProceduralGeneration min=0 step=0.01");
	TwAddVarRW(m_generalBar, "Position_X",		TW_TYPE_FLOAT, &m_pPos.x,		"group=ProceduralGeneration step=0.01");
	TwAddVarRW(m_generalBar, "Position_Y",		TW_TYPE_FLOAT, &m_pPos.y,		"group=ProceduralGeneration step=0.01");
	TwAddVarRW(m_generalBar, "Position_Z",		TW_TYPE_FLOAT, &m_pPos.z,		"group=ProceduralGeneration step=0.01");
	TwAddVarRW(m_generalBar, "Scale_X",			TW_TYPE_FLOAT, &m_pScale.x,		"group=ProceduralGeneration min=0 step=0.01");
	TwAddVarRW(m_generalBar, "Scale_Y",			TW_TYPE_FLOAT, &m_pScale.y,		"group=ProceduralGeneration min=0 step=0.01");
	TwAddVarRW(m_generalBar, "Scale_Z",			TW_TYPE_FLOAT, &m_pScale.z,		"group=ProceduralGeneration min=0 step=0.01");
	TwAddVarRW(m_generalBar, "Rotation",		TW_TYPE_QUAT4F,&m_pRot,			"group=ProceduralGeneration");
	//Only the scale variable affects simulation in realtime. All others require recreating the perlinTexture.

	m_cloths.push_back(new ClothData(4, 4, 8, "./data/textures/crate.png"));
	AddCloth(PhysScene::AddCloth(vec3(0, 40, 0), m_cloths.back()->m_vertexCount, m_cloths.back()->m_indexCount, m_cloths.back()->m_vertices, m_cloths.back()->m_indices));

	return true;
}

bool VirtualWorld::shutdown(){
	//Game Asset Termination
	delete m_player;
	delete m_navi;

	//Clear all pointer arrays of their stored memory.
	for (GPUEmitter* particle : m_particleEmitters) { delete particle; }
	for (FBXModel* model : m_FBXModels) { delete model; }
	for (ClothData* cloth : m_cloths) { delete cloth; }
	for (DirectionalLight* light : m_dirLights) { delete light; }
	for (PointLight* light : m_pointLights) { delete light; }
	for (Sprite* sprite : m_pointLightSprites) { delete sprite; }
		
	glDeleteFramebuffers(1, &m_fxFBO);
	glDeleteFramebuffers(1, &m_gBufferFBO);
	glDeleteFramebuffers(1, &m_lightFBO);

	glDeleteTextures(1, &m_depthTexture);
	glDeleteTextures(1, &m_albedoTexture);
	glDeleteTextures(1, &m_normalTexture);
	glDeleteTextures(1, &m_positionTexture);
	glDeleteTextures(1, &m_specularTexture);

	glDeleteTextures(1, &m_lightTexture);
	glDeleteTextures(1, &m_fxTexture);

	glDeleteTextures(1, &m_pHeightTexture);
	glDeleteTextures(1, &m_pTexture);

	//Delete Buffers for deferred rendering quad and lightcube.
	glDeleteVertexArrays(1, &m_screenspaceQuad.m_VAO);
	glDeleteBuffers(1, &m_screenspaceQuad.m_VBO);
	glDeleteVertexArrays(1, &m_lightCube.m_VAO);
	glDeleteBuffers(1, &m_lightCube.m_VBO);

	//Shader Termination
	glDeleteProgram(m_gBufferProgram);
	glDeleteProgram(m_compositeProgram);
	glDeleteProgram(m_dirLightProgram);
	glDeleteProgram(m_pointLightProgram);
	glDeleteProgram(m_proceduralProgram);

	PhysScene::Shutdown();

	//GUI Termination
	TwTerminate(); //End ATB
	Gizmos::destroy(); //End Gizmos

	return Application::shutdown();
}


bool VirtualWorld::update(){
	if (!Application::update()){
		return false;
	}
	Gizmos::clear();

	//Clamping deltaTime to 1/30 if it exceeds that, to stop crazy things from happening
	m_fDeltaTime = (m_fDeltaTime > 1.0f / 30.f) ? 1.0f / 30.0f : m_fDeltaTime;

	//Button Input for shader reloading. Uses lastKey variable to only pass the first frame the key is pressed.
	if (glfwGetKey(m_window, GLFW_KEY_R) == GLFW_PRESS && m_lastKey[0] != GLFW_PRESS){
		ReloadShaders();
	}
	if (glfwGetKey(m_window, GLFW_KEY_T) == GLFW_PRESS && m_lastKey[1] != GLFW_PRESS) {
		BuildPerlinTexture(glm::ivec2(256, 256), m_pOct, m_pAmp, m_pPers);
	}

	//Update last key inputs, to prevent the previous checks passing every frame.
	m_lastKey[0] = glfwGetKey(m_window, GLFW_KEY_R);
	m_lastKey[1] = glfwGetKey(m_window, GLFW_KEY_T);

	PhysScene::Update(m_fDeltaTime, m_debug[2]);
	m_oCamera.Update(m_fDeltaTime, m_player->GetPos());

	for (FBXModel* model : m_FBXModels) {
		model->Update(m_fDeltaTime);
	}

	for (AIEntity* ent : m_AIEntities) {
		ent->SetTarget(m_player->GetPos());
		ent->Update(m_fDeltaTime, m_player->GetPos());
	}

	if (m_debug[6]) {
		for (unsigned int i = 0; i < 10; i++) {
			Gizmos::addSphereFilled(m_nodes[i].m_pos, 1.0f, 6,6, vec4(vec3((float)i/10), 1));
			Gizmos::addLine(m_nodes[i].m_pos, m_nodes[i].m_next->m_pos, vec4(1, 1, 0, 1), vec4(1, 1, 0, 1));
		}
	}

	for (unsigned int i = 0; i < m_pointLights.size(); i++) {
		m_pointLightSprites[i]->m_pos = m_pointLights[i]->m_pos;
		m_pointLightSprites[i]->m_maxColor = m_pointLights[i]->m_color;
		m_pointLightSprites[i]->m_minColor = m_pointLights[i]->m_color;
		m_pointLightSprites[i]->m_maxSize = m_pointLights[i]->m_radius / 2.5f;
		m_pointLightSprites[i]->m_minSize = m_pointLights[i]->m_radius / 2.5f;
		m_pointLightSprites[i]->Update(m_fDeltaTime, m_oCamera.GetWorldTransform());
	}

	m_player->Update(m_fDeltaTime, m_oCamera.GetWorldTransform());
	m_navi->Update(m_fDeltaTime, m_oCamera.GetWorldTransform(), m_player->GetPos());

	//Parenting wispy particles to Navi
	m_particleEmitters[0]->m_pos = m_navi->GetPos();
	m_particleEmitters[0]->m_startColor = vec4(m_navi->GetColor(), 1);
	m_particleEmitters[0]->m_endColor = vec4(m_navi->GetColor(), 1);
	//Parenting floaty particles to the camera
	m_particleEmitters[1]->m_pos = m_oCamera.GetWorldTransform()[3].xyz;

	//Set the stuck vertex of the cloth to the player's position.
	m_cloths.back()->m_nailPos = m_player->GetPos();

	//Defining Gizmos debug colors.
	vec4 white(1);
	vec4 black(0.3, 0.3, 0.3, 1); //Black not pure black, as deferred rendering requires a clearcolor of pure black.

	//Grid construction.
	if (m_debug[1]) {
		Gizmos::addTransform(mat4(1), 10);
		for (int i = 0; i <= 20; ++i){
			Gizmos::addLine(vec3(-10 + i, 0, -10), i != 10 ? vec3(-10 + i, 0, 10) : vec3(-10 + i, 0, 0), i != 10 ? black : white);
			Gizmos::addLine(vec3(-10, 0, -10 + i), i != 10 ? vec3(10, 0, -10 + i) : vec3(0, 0, -10 + i), i != 10 ? black : white);
		}
	}
	return true;
}


void VirtualWorld::draw(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST); //Enable Depth Culling.
	glEnable(GL_CULL_FACE); //Enable Face Culling of polygons facing away from camera.

	glDepthFunc(GL_LESS); //Only draw objects if they're closer to the camera	

	//Shadow Depth Rendering
	glViewport(0, 0, 4096, 4096); //Set viewport to texture size
	for (DirectionalLight* light : m_dirLights) {
		glBindFramebuffer(GL_FRAMEBUFFER, light->m_shadowFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		//Create a projection view matrix for the light
		mat4 lightView = glm::lookAt(vec3(0), light->m_dir, vec3(0, 1, 0));
		mat4 lightMatrix = DirectionalLight::m_lightProj * lightView * mat4(1);

		//Models have specialised shadow handling; no need to use one in the main loop
		for (FBXModel* model : m_FBXModels){
			//Pass the lightMatrix in as an override of the camera projection view matrix
			model->Render(&m_oCamera, false, &lightMatrix);
		}
	}

	glViewport(0, 0, 512, 512); //Viewport to texture size
	for (PointLight* light : m_pointLights) {
		for (int i = 0; i < 6; i++) {
			glBindFramebuffer(GL_FRAMEBUFFER, light->m_shadowFBO);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, PointLight::m_cubemapDirection[i].CubemapFace, light->m_shadowFBO, 0);
			glDrawBuffer(GL_NONE);

			glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

			mat4 lightView = glm::lookAt(PointLight::m_cubemapDirection[i].Up, PointLight::m_cubemapDirection[i].Target, vec3(0, 1, 0));
			mat4 lightMatrix = PointLight::m_lightProj * lightView * mat4(1);

			for (FBXModel* model : m_FBXModels){
				model->Render(&m_oCamera, false, &lightMatrix);
			}
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, m_gBufferFBO);
	glViewport(0, 0, m_iWidth, m_iHeight);
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearBufferfv(GL_COLOR, 0, (float*)&vec4(0.0f));
	glClearBufferfv(GL_COLOR, 1, (float*)&vec4(0.0f));
	glClearBufferfv(GL_COLOR, 2, (float*)&vec4(0.5f));

	//glDepthMask(GL_FALSE);
	glUseProgram(m_proceduralProgram);

	int loc = glGetUniformLocation(m_proceduralProgram, "view");
	glUniformMatrix4fv(loc, 1, GL_FALSE, (float*)&m_oCamera.GetView());

	loc = glGetUniformLocation(m_proceduralProgram, "projView");
	glUniformMatrix4fv(loc, 1, GL_FALSE, (float*)&m_oCamera.GetProjectionView());

	m_pTransform = glm::translate(m_pPos) * glm::scale(m_pScale) * glm::toMat4(m_pRot);
	loc = glGetUniformLocation(m_proceduralProgram, "transform");
	glUniformMatrix4fv(loc, 1, GL_FALSE, (float*)&m_pTransform);

	loc = glGetUniformLocation(m_proceduralProgram, "scale");
	glUniform1f(loc, m_pHeightScale);

	loc = glGetUniformLocation(m_proceduralProgram, "perlinTexture");
	glUniform1i(loc, 0);

	loc = glGetUniformLocation(m_proceduralProgram, "diffuse");
	glUniform1i(loc, 1);

	loc = glGetUniformLocation(m_proceduralProgram, "roughness");
	glUniform1f(loc, m_pRoughness);

	loc = glGetUniformLocation(m_proceduralProgram, "fresnelScale");
	glUniform1f(loc, m_pFresnelScale);

	loc = glGetUniformLocation(m_proceduralProgram, "textureScale");
	glUniform1f(loc, m_pTextureScale);

	loc = glGetUniformLocation(m_proceduralProgram, "worldSize");
	glUniform2fv(loc, 1, (float*)&m_pWorldSize);

	loc = glGetUniformLocation(m_proceduralProgram, "textureSize");
	glUniform2fv(loc, 1, (float*)&m_pTextureSize);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_pHeightTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_pTexture);

	glBindVertexArray(m_planeMesh.m_VAO);
	glDrawElements(GL_TRIANGLES, m_planeMesh.m_indexCount, GL_UNSIGNED_INT, 0);

	glUseProgram(m_gBufferProgram);

	loc = glGetUniformLocation(m_gBufferProgram, "view");
	glUniformMatrix4fv(loc, 1, GL_FALSE, (float*)&m_oCamera.GetView());

	loc = glGetUniformLocation(m_gBufferProgram, "projView");
	glUniformMatrix4fv(loc, 1, GL_FALSE, (float*)&m_oCamera.GetProjectionView());

	//Opaque Drawing

	for (ClothData* cloth : m_cloths) {
		cloth->Render(&m_oCamera);
	}
	for (FBXModel* model : m_FBXModels){
		model->Render(&m_oCamera, true);
	}

	//Light Rendering
	glBindFramebuffer(GL_FRAMEBUFFER, m_lightFBO);
	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_BLEND);//Enable transparency blending

	
	//RenderDirectionalLight(vec3(1, 0, 0), vec3(1, 0, 0));
	//RenderDirectionalLight(vec3(0, 1, 0), vec3(0, 1, 0));
	//RenderDirectionalLight(vec3(0, 0, 1), vec3(0, 0, 1));
	//RenderDirectionalLight(vec3(0, -1, 0), vec3(1));
	//RenderDirectionalLight(vec3(0, 0, -1), vec3(0.7));
	RenderDirectionalLights();
	RenderPointLights();

	//Effects Rendering
	glBindFramebuffer(GL_FRAMEBUFFER, m_fxFBO);
	glClear(GL_COLOR_BUFFER_BIT);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_FALSE); //Disable newly drawn objects writing to the depth buffer. Allows all FX to be drawn without causing culling.
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glUseProgram(m_gBufferProgram);
	for (unsigned int i = 0; i < m_particleEmitters.size(); i++) {
		m_particleEmitters[i]->Render(m_fCurrTime, m_oCamera, true);
		if (m_debug[3]) {
			m_particleEmitters[i]->RenderGizmos();
		}
	}

	for (Sprite* sprite : m_pointLightSprites) {
		sprite->Render(&m_oCamera);
	}
	//Render navi's sprite
	m_navi->RenderSprite(&m_oCamera);

	//Draw Gizmos at the end of the transparency pass, to retain depth culling without lighting.
	if (!m_debug[0]) {
		Gizmos::draw(m_oCamera.GetProjectionView());
	}

	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND); //No more transparent rendering, turn off the blend.
	glBlendFunc(GL_ONE, GL_ONE);

	//Composite Rendering
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(0.3f, 0.3f, 0.3f, 1);

	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(m_compositeProgram);

	glUniform1i(glGetUniformLocation(m_compositeProgram, "albedoTexture"), 0);
	glUniform1i(glGetUniformLocation(m_compositeProgram, "lightTexture"),  1);
	glUniform1i(glGetUniformLocation(m_compositeProgram, "fxTexture"), 2);
	glUniform1i(glGetUniformLocation(m_compositeProgram, "shadowMap"), 3);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_albedoTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_lightTexture);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_fxTexture);
	if (m_pointLights.size() > 0) {
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_pointLights[0]->m_shadowMap);
	}

	loc = glGetUniformLocation(m_compositeProgram, "ambCol");
	glUniform3fv(loc, 1, (float*)&m_ambCol);

	glDepthFunc(GL_ALWAYS);//Interface data / The screenspace quad should always be drawn.

	//Render the screen texture to the screen quad.
	glBindVertexArray(m_screenspaceQuad.m_VAO);
	glDrawElements(GL_TRIANGLES, m_screenspaceQuad.m_indexCount, GL_UNSIGNED_INT, 0);

	//GUI and Interface Drawing.

	//Gizmos without any depth culling.
	if (m_debug[0]) {
		Gizmos::draw(m_oCamera.GetProjectionView());
	}

	TwDraw();

	Application::draw();
}

void VirtualWorld::RenderDirectionalLights() {
	glUseProgram(m_dirLightProgram);

	//Tell the shader which slot holds each texture
	glUniform1i(glGetUniformLocation(m_dirLightProgram, "positionTexture"), 0);
	glUniform1i(glGetUniformLocation(m_dirLightProgram, "normalTexture"), 1);
	glUniform1i(glGetUniformLocation(m_dirLightProgram, "specularTexture"), 2);
	glUniform1i(glGetUniformLocation(m_dirLightProgram, "shadowMap"), 3);

	//Bind the textures to the slots
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_positionTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_normalTexture);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_specularTexture);

	int loc = glGetUniformLocation(m_dirLightProgram, "projView");
	glUniformMatrix4fv(loc, 1, GL_FALSE, (float*)&m_oCamera.GetProjectionView());
	loc = glGetUniformLocation(m_dirLightProgram, "world");
	glUniformMatrix4fv(loc, 1, GL_FALSE, (float*)&m_oCamera.GetWorldTransform());

	for (DirectionalLight* light : m_dirLights) {
		light->Render(&m_oCamera, m_dirLightProgram);
		if (m_debug[5]) {
			Gizmos::addSphere(-light->m_dir * 15000, 350, 8, 8, vec4(light->m_color, 0.5));
		}
		glBindVertexArray(m_screenspaceQuad.m_VAO);
		glDrawElements(GL_TRIANGLES, m_screenspaceQuad.m_indexCount, GL_UNSIGNED_INT, 0);
	}
}

void VirtualWorld::RenderPointLights() {
	glUseProgram(m_pointLightProgram);

	glUniform1i(glGetUniformLocation(m_pointLightProgram, "positionTexture"), 0);
	glUniform1i(glGetUniformLocation(m_pointLightProgram, "normalTexture"), 1);
	glUniform1i(glGetUniformLocation(m_pointLightProgram, "specularTexture"), 2);
	glUniform1i(glGetUniformLocation(m_pointLightProgram, "shadowMap"), 3);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_positionTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_normalTexture);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_specularTexture);

	int loc = glGetUniformLocation(m_pointLightProgram, "projView");
	glUniformMatrix4fv(loc, 1, GL_FALSE, (float*)&m_oCamera.GetProjectionView());
	loc = glGetUniformLocation(m_pointLightProgram, "world");
	glUniformMatrix4fv(loc, 1, GL_FALSE, (float*)&m_oCamera.GetWorldTransform());

	//Render the navi light.
	m_navi->RenderLight(&m_oCamera, m_pointLightProgram);
	glBindVertexArray(m_lightCube.m_VAO);
	glDrawElements(GL_TRIANGLES, m_lightCube.m_indexCount, GL_UNSIGNED_INT, 0);

	for (PointLight* light : m_pointLights) {
		light->Render(&m_oCamera, m_pointLightProgram);
		if (m_debug[4]) {
			Gizmos::addAABB(light->m_pos, vec3(0.5), vec4(light->m_color, 1));
		}

		glBindVertexArray(m_lightCube.m_VAO);
		glDrawElements(GL_TRIANGLES, m_lightCube.m_indexCount, GL_UNSIGNED_INT, 0);
	}
}

//Convenience function to initialise directional light data within simulation and add it to GUI.
void VirtualWorld::AddDirectionalLight(vec3 a_dir, vec3 a_color) {
	m_dirLights.push_back(new DirectionalLight(a_dir, a_color));

	TwBar* m_lightingBar = TwGetBarByName("Lighting");

	if (m_lightingBar == nullptr) return; //No Lighting bar was found.

	//Formatting the light data with a unique prefix, so AntTweakBar can display it. ATB rejects duplicate keyvalue names.
	std::string prefix("DL" + std::to_string(m_dirLights.size()) + "_");
	std::string group = std::string("group=DirectionalLight" + std::to_string(m_dirLights.size()));

	TwAddVarRW(m_lightingBar, std::string(prefix + "Direction").c_str(), TW_TYPE_DIR3F, &m_dirLights.back()->m_dir, group.c_str());
	TwAddVarRW(m_lightingBar, std::string(prefix + "Color").c_str(), TW_TYPE_COLOR3F, &m_dirLights.back()->m_color, group.c_str());

}

//Convenience function that creates point light data and adds it to the lighting GUI window.
void VirtualWorld::AddPointLight(vec3 a_pos, vec3 a_color, float a_radius) {
	m_pointLights.push_back(new PointLight(a_pos, a_color, a_radius));
	m_pointLightSprites.push_back(new Sprite(a_pos, 20, a_color, "./data/textures/particles/glow2.png"));

	TwBar* m_lightingBar = TwGetBarByName("Lighting");

	if (m_lightingBar == nullptr) return; //No Lighting bar was found.

	std::string prefix("PL" + std::to_string(m_pointLights.size()) + "_");
	std::string group = std::string("group=PointLight" + std::to_string(m_pointLights.size()));

	TwAddVarRW(m_lightingBar, std::string(prefix + "X").c_str(), TW_TYPE_FLOAT, &m_pointLights.back()->m_pos.x, (std::string("step=0.1 ") + group).c_str());
	TwAddVarRW(m_lightingBar, std::string(prefix + "Y").c_str(), TW_TYPE_FLOAT, &m_pointLights.back()->m_pos.y, (std::string("step=0.1 ") + group).c_str());
	TwAddVarRW(m_lightingBar, std::string(prefix + "Z").c_str(), TW_TYPE_FLOAT, &m_pointLights.back()->m_pos.z, (std::string("step=0.1 ") + group).c_str());
	TwAddVarRW(m_lightingBar, std::string(prefix + "Color").c_str(), TW_TYPE_COLOR3F, &m_pointLights.back()->m_color, group.c_str());
	TwAddVarRW(m_lightingBar, std::string(prefix + "Radius").c_str(), TW_TYPE_FLOAT, &m_pointLights.back()->m_radius, (std::string("min=0 ") + group).c_str());
}

//Convenience function that add an FBXModel to the vector array, while also adding it to the Model GUI window.
void VirtualWorld::AddFBXModel(FBXModel* a_model) {
	for (auto model : m_FBXModels) {
		if (model == a_model)
			return;
	}
	m_FBXModels.push_back(a_model);

	TwBar* m_modelBar = TwGetBarByName("Models");

	if (m_modelBar == nullptr) return; //No Models bar was found.

	std::string prefix(std::to_string(m_FBXModels.size()) + "_");
	std::string group = std::string("group=Model" + std::to_string(m_FBXModels.size()));

	TwAddVarRW(m_modelBar, std::string(prefix + "Position_X").c_str(), TW_TYPE_FLOAT, &m_FBXModels.back()->m_pos.x, (std::string("step=0.1 ") + group).c_str());
	TwAddVarRW(m_modelBar, std::string(prefix + "Position_Y").c_str(), TW_TYPE_FLOAT, &m_FBXModels.back()->m_pos.y, (std::string("step=0.1 ") + group).c_str());
	TwAddVarRW(m_modelBar, std::string(prefix + "Position_Z").c_str(), TW_TYPE_FLOAT, &m_FBXModels.back()->m_pos.z, (std::string("step=0.1 ") + group).c_str());
	TwAddVarRW(m_modelBar, std::string(prefix + "Scale_X").c_str(), TW_TYPE_FLOAT, &m_FBXModels.back()->m_scale.x, (std::string("min=0.01 step=0.01 ") + group).c_str());
	TwAddVarRW(m_modelBar, std::string(prefix + "Scale_Y").c_str(), TW_TYPE_FLOAT, &m_FBXModels.back()->m_scale.y, (std::string("min=0.01 step=0.01 ") + group).c_str());
	TwAddVarRW(m_modelBar, std::string(prefix + "Scale_Z").c_str(), TW_TYPE_FLOAT, &m_FBXModels.back()->m_scale.z, (std::string("min=0.01 step=0.01 ") + group).c_str());
	TwAddVarRW(m_modelBar, std::string(prefix + "Rotation").c_str(), TW_TYPE_QUAT4F, &m_FBXModels.back()->m_rot, group.c_str());
	TwAddVarRW(m_modelBar, std::string(prefix + "Roughness").c_str(), TW_TYPE_FLOAT, &m_FBXModels.back()->m_roughness, (std::string("min=0 step=0.05 max=100.0 ") + group).c_str());
	TwAddVarRW(m_modelBar, std::string(prefix + "Fresnel_Scale").c_str(), TW_TYPE_FLOAT, &m_FBXModels.back()->m_fresnelScale, (std::string("min=0 step=0.05 max=100.0 ") + group).c_str());
}

void VirtualWorld::AddParticleEmitter(GPUEmitter* a_particle) {
	m_particleEmitters.push_back(a_particle);

	TwBar* m_particlesBar = TwGetBarByName("Particles");

	if (m_particlesBar == nullptr) return; //No Particles bar was found.

	std::string prefix(std::to_string(m_particleEmitters.size()) + "_");
	std::string group = std::string("group=ParticleEmitter" + std::to_string(m_particleEmitters.size()));

	TwEnumVal emitTypes[] = { { EMIT_POINT, "Point" }, { EMIT_LINE, "Line" }, { EMIT_PLANE, "Plane" }, { EMIT_RING, "Ring" }, { EMIT_OUTER_RING, "Outer Ring" },
	{ EMIT_RECTANGLE, "Rectangle" }, { EMIT_OUTER_RECTANGLE, "Outer Rectangle" }, { EMIT_SPHERE, "Sphere" }, { EMIT_OUTER_SPHERE, "Outer Sphere" } };
	TwType emitType = TwDefineEnum("EmitType", emitTypes, 9);

	TwEnumVal moveTypes[] = { { PMOVE_LINEAR, "Linear" }, { PMOVE_WAVE, "Wave" } };
	TwType moveType = TwDefineEnum("MoveType", moveTypes, 2);

	TwAddVarRW(m_particlesBar, std::string(prefix + "Position_X").c_str(), TW_TYPE_FLOAT, &m_particleEmitters.back()->m_pos.x, (std::string("step=0.1 min=0 ") + group).c_str());
	TwAddVarRW(m_particlesBar, std::string(prefix + "Position_Y").c_str(), TW_TYPE_FLOAT, &m_particleEmitters.back()->m_pos.y, (std::string("step=0.1 min=0 ") + group).c_str());
	TwAddVarRW(m_particlesBar, std::string(prefix + "Position_Z").c_str(), TW_TYPE_FLOAT, &m_particleEmitters.back()->m_pos.z, (std::string("step=0.1 min=0 ") + group).c_str());
	TwAddVarRW(m_particlesBar, std::string(prefix + "Extents_X").c_str(), TW_TYPE_FLOAT, &m_particleEmitters.back()->m_extents.x, (std::string("step=0.1 min=0 ") + group).c_str());
	TwAddVarRW(m_particlesBar, std::string(prefix + "Extents_Y").c_str(), TW_TYPE_FLOAT, &m_particleEmitters.back()->m_extents.y, (std::string("step=0.1 min=0 ") + group).c_str());
	TwAddVarRW(m_particlesBar, std::string(prefix + "Extents_Z").c_str(), TW_TYPE_FLOAT, &m_particleEmitters.back()->m_extents.z, (std::string("step=0.1 min=0 ") + group).c_str());
	TwAddVarRW(m_particlesBar, std::string(prefix + "Emit_Type").c_str(), emitType, &m_particleEmitters.back()->m_emitType, group.c_str());
	TwAddVarRW(m_particlesBar, std::string(prefix + "Move_Type").c_str(), moveType, &m_particleEmitters.back()->m_moveType, group.c_str());
	TwAddVarRW(m_particlesBar, std::string(prefix + "Lifespan_Max").c_str(), TW_TYPE_FLOAT, &m_particleEmitters.back()->m_lifespanMax, (std::string("step=0.05 min=0 ") + group).c_str());
	TwAddVarRW(m_particlesBar, std::string(prefix + "Lifespan_Min").c_str(), TW_TYPE_FLOAT, &m_particleEmitters.back()->m_lifespanMin, (std::string("step=0.05 min=0 ") + group).c_str());
	TwAddVarRW(m_particlesBar, std::string(prefix + "Velocity_Min").c_str(), TW_TYPE_FLOAT, &m_particleEmitters.back()->m_velocityMin, (std::string("step=0.05 min=0 ") + group).c_str());
	TwAddVarRW(m_particlesBar, std::string(prefix + "Velocity_Max").c_str(), TW_TYPE_FLOAT, &m_particleEmitters.back()->m_velocityMax, (std::string("step=0.05 min=0 ") + group).c_str());
	TwAddVarRW(m_particlesBar, std::string(prefix + "Fade_In").c_str(), TW_TYPE_FLOAT, &m_particleEmitters.back()->m_fadeIn, (std::string("step=0.05 min=0 ") + group).c_str());
	TwAddVarRW(m_particlesBar, std::string(prefix + "Fade_Out").c_str(), TW_TYPE_FLOAT, &m_particleEmitters.back()->m_fadeOut, (std::string("step=0.05 min=0 ") + group).c_str());
	TwAddVarRW(m_particlesBar, std::string(prefix + "Start_Size").c_str(), TW_TYPE_FLOAT, &m_particleEmitters.back()->m_startSize, (std::string("step=0.05 min=0 ") + group).c_str());
	TwAddVarRW(m_particlesBar, std::string(prefix + "End_Size").c_str(), TW_TYPE_FLOAT, &m_particleEmitters.back()->m_endSize, (std::string("step=0.05 min=0 ") + group).c_str());
	TwAddVarRW(m_particlesBar, std::string(prefix + "Start_Color").c_str(), TW_TYPE_COLOR4F, &m_particleEmitters.back()->m_startColor, group.c_str());
	TwAddVarRW(m_particlesBar, std::string(prefix + "End_Color").c_str(), TW_TYPE_COLOR4F, &m_particleEmitters.back()->m_endColor, group.c_str());
}

void VirtualWorld::AddCloth(PxCloth* a_cloth) {
	m_cloths.back()->m_cloth = a_cloth;
	m_cloths.back()->GenerateGLBuffers();
}

//Build the screenspace quad that will display the final render graphic.
void VirtualWorld::BuildQuad() {
	vec2 half_texel = 1.0f / vec2(m_iWidth, m_iHeight) * 0.5f;

	float vertexData[] {
		-1, -1, 0, 1, 0 + half_texel.x, 0 + half_texel.y,
		 1, -1, 0, 1, 1 - half_texel.x, 0 + half_texel.y,
		 1,  1, 0, 1, 1 - half_texel.x, 1 - half_texel.y,
		-1,  1, 0, 1, 0 + half_texel.x, 1 - half_texel.y,
	};

	unsigned int indexData[] = { 0, 1, 2, 0, 2, 3 };

	m_screenspaceQuad.m_indexCount = 6;

	glGenVertexArrays(1, &m_screenspaceQuad.m_VAO);

	glGenBuffers(1, &m_screenspaceQuad.m_VBO);
	glGenBuffers(1, &m_screenspaceQuad.m_IBO);

	glBindVertexArray(m_screenspaceQuad.m_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, m_screenspaceQuad.m_VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_screenspaceQuad.m_IBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexData), indexData, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(float)* 6, 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float)* 6, (void*)(sizeof(float)* 4));

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

//Build the light cube that will hold point light data.
void VirtualWorld::BuildCube() {
	float vertexData[] {
		-1, -1,  1,  1,
		 1, -1,  1,  1,
		 1, -1, -1,  1,
		-1, -1, -1,  1,

		-1,  1,  1,  1,
		 1,  1,  1,  1,
		 1,  1, -1,  1,
		-1,  1, -1,  1
	};

	unsigned int indexData[] = {
		4, 5, 0,
		5, 1, 0,
		5, 6, 1,
		6, 2, 1,
		6, 7, 2,
		7, 3, 2,
		7, 4, 3,
		4, 0, 3,
		7, 6, 4,
		6, 5, 4,
		0, 1, 3,
		1, 2, 3
	};

	m_lightCube.m_indexCount = 36;

	glGenVertexArrays(1, &m_lightCube.m_VAO);
	glBindVertexArray(m_lightCube.m_VAO);

	glGenBuffers(1, &m_lightCube.m_VBO);
	glGenBuffers(1, &m_lightCube.m_IBO);


	glBindBuffer(GL_ARRAY_BUFFER, m_lightCube.m_VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_lightCube.m_IBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexData), indexData, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(float)* 4, 0);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void VirtualWorld::BuildFrameBuffers() {
	//Geometry Framebuffer
	glGenFramebuffers(1, &m_gBufferFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_gBufferFBO);

	//Albedo only needs 3 channels, the others need 4 for extra data.
	glGenTextures(1, &m_albedoTexture);
	glBindTexture(GL_TEXTURE_2D, m_albedoTexture);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB8, m_iWidth, m_iHeight);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glGenTextures(1, &m_positionTexture);
	glBindTexture(GL_TEXTURE_2D, m_positionTexture);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, m_iWidth, m_iHeight);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glGenTextures(1, &m_normalTexture);
	glBindTexture(GL_TEXTURE_2D, m_normalTexture);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, m_iWidth, m_iHeight);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glGenTextures(1, &m_specularTexture);
	glBindTexture(GL_TEXTURE_2D, m_specularTexture);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, m_iWidth, m_iHeight);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glGenRenderbuffers(1, &m_depthTexture);
	glBindRenderbuffer(GL_RENDERBUFFER, m_depthTexture);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_iWidth, m_iHeight);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_albedoTexture, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, m_positionTexture, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, m_normalTexture, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, m_specularTexture, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthTexture);

	GLenum gPassTargets[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
	glDrawBuffers(4, gPassTargets);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
		printf("Error creating gbuffer!\n");

	//Light Framebuffer
	glGenFramebuffers(1, &m_lightFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_lightFBO);

	glGenTextures(1, &m_lightTexture);
	glBindTexture(GL_TEXTURE_2D, m_lightTexture);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA16F, m_iWidth, m_iHeight);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_lightTexture, 0);

	GLenum lightTargets[] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, lightTargets);

	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
		printf("Error creating light framebuffer!\n");

	//Effects Framebuffer
	glGenFramebuffers(1, &m_fxFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_fxFBO);

	glGenTextures(1, &m_fxTexture);
	glBindTexture(GL_TEXTURE_2D, m_fxTexture);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB8, m_iWidth, m_iHeight);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_fxTexture, 0);
	//Pass the depth buffer from the geometry pass into the FX framebuffer, so depth culling is done correctly.
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthTexture);

	GLenum fxTargets[] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, fxTargets);

	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
		printf("Error creating light framebuffer!\n");

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void VirtualWorld::BuildProceduralGrid(vec2 a_realDims, glm::ivec2 a_dims){
	m_pWorldSize = a_realDims;
	m_pTextureSize = a_dims;

	int vertexCount = (a_dims.x + 1) * (a_dims.y + 1);

	VertexTexCoord* vertexData = new VertexTexCoord[vertexCount];

	unsigned int indexCount = a_dims.x * a_dims.y * 6;
	unsigned int* indexData = new unsigned int[indexCount];

	float currY = -a_realDims.y / 2.0f;
	for (int y = 0; y < a_dims.y + 1; ++y){
		float currX = -a_realDims.x / 2.0f;
		for (int x = 0; x < a_dims.x + 1; ++x){
			vertexData[y * (a_dims.x + 1) + x].position = vec4(currX, 0, currY, 1);
			vertexData[y * (a_dims.x + 1) + x].tex_coord = vec2((float)x / (float)a_dims.x, (float)y / (float)a_dims.y);
			currX += a_realDims.x / (float)a_dims.x;
		}
		currY += a_realDims.y / (float)a_dims.y;
	}

	int currIndex = 0;
	for (int y = 0; y < a_dims.y; ++y){
		for (int x = 0; x < a_dims.x; ++x){
			indexData[currIndex++] = y		 * (a_dims.x + 1) + x;
			indexData[currIndex++] = (y + 1) * (a_dims.x + 1) + x;
			indexData[currIndex++] = (y + 1) * (a_dims.x + 1) + (x + 1);

			indexData[currIndex++] = (y + 1) * (a_dims.x + 1) + (x + 1);
			indexData[currIndex++] = y		 * (a_dims.x + 1) + (x + 1);
			indexData[currIndex++] = y		 * (a_dims.x + 1) + x;
		}
	}

	m_planeMesh.m_indexCount = indexCount;

	glGenVertexArrays(1, &m_planeMesh.m_VAO);
	glGenBuffers(1, &m_planeMesh.m_VBO);
	glGenBuffers(1, &m_planeMesh.m_IBO);

	glBindVertexArray(m_planeMesh.m_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_planeMesh.m_VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_planeMesh.m_IBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(VertexTexCoord)* vertexCount, vertexData, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)* indexCount, indexData, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(VertexTexCoord), 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexTexCoord), (void*)sizeof(vec4));

	delete[] vertexData;
	delete[] indexData;
}

void VirtualWorld::BuildPerlinTexture(glm::ivec2 a_dims, const int a_octaves, const float a_amplitude, const float a_persistance) {
	float scale = (1.0f / a_dims.x) * 5.0f;

	m_pData = new float[a_dims.x * a_dims.y];

	for (int y = 0; y < a_dims.y; y++) {
		for (int x = 0; x < a_dims.x; x++) {

			float amplitude = a_amplitude;
			float freq = 1;

			m_pData[y*a_dims.x + x] = 0;

			for (int o = 0; o < a_octaves; o++) {
				float perlinSample = glm::perlin(vec2((float)x, (float)y) * scale * freq) * 0.5f + 0.5f;

				perlinSample *= amplitude;
				m_pData[y*a_dims.x + x] += perlinSample;

				amplitude *= a_persistance;
				freq *= 2;
			}
		}
	}

	glGenTextures(1, &m_pHeightTexture);
	glBindTexture(GL_TEXTURE_2D, m_pHeightTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, a_dims.x, a_dims.y, 0, GL_RED, GL_FLOAT, m_pData);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

//Function to reload shaders during run-time.
void VirtualWorld::ReloadShaders(){
	printf("Reloading Shaders...\n");

	glDeleteProgram(m_gBufferProgram);
	glDeleteProgram(m_compositeProgram);
	glDeleteProgram(m_dirLightProgram);
	glDeleteProgram(m_pointLightProgram);
	glDeleteProgram(m_proceduralProgram);

	LoadShader("./data/shaders/gbuffer.vs", 0, "./data/shaders/gbuffer.fs", &m_gBufferProgram);
	LoadShader("./data/shaders/perlin.vs", 0, "./data/shaders/gbuffer_textured.fs", &m_proceduralProgram);
	LoadShader("./data/shaders/composite.vs", 0, "./data/shaders/composite.fs", &m_compositeProgram);
	LoadShader("./data/shaders/directional_light.vs", 0, "./data/shaders/directional_light.fs", &m_dirLightProgram);
	LoadShader("./data/shaders/point_light.vs", 0, "./data/shaders/point_light.fs", &m_pointLightProgram);

	for (GPUEmitter* particle : m_particleEmitters) {
		particle->Reload();
	}

	for (FBXModel* model : m_FBXModels) {
		model->ReloadShader();
	}
}

void VirtualWorld::resize(int a_width, int a_height){
	glViewport(0, 0, a_width, a_height);
	TwWindowSize(a_width, a_height);
	m_iWidth = a_width;
	m_iHeight = a_height;
	BuildFrameBuffers();
	BuildQuad();
}