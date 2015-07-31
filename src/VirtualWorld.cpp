#include "VirtualWorld.h"
#include "AntTweakBar.h"
#include <string>
#include "stb_image.h"

VirtualWorld::VirtualWorld(): m_oCamera(50), m_pScale(50.f), m_pOct(6), m_pAmp(0.9f), m_pPers(0.2f), m_ambCol(vec3(0.2f)){
	Application::Application();
}
VirtualWorld::~VirtualWorld(){}

//Virtual world app stored as a global variable. Mainly for AntTweakBar to access the application during button callbacks.
VirtualWorld* m_app;

bool VirtualWorld::startup(){
	if (!Application::startup()){
		return false;
	}
	m_app = this;

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
	m_oCamera.setPerspective(glm::radians(50.0f), 1280.0f / 720.0f, 0.1f, 20000.0f);

	//Shader Initialisation
	//Opaque Geometry Shaders
	LoadShader("./data/shaders/gbuffer_vertex.glsl", 0, "./data/shaders/gbuffer_fragment.glsl", &m_gBufferProgram);
	LoadShader("./data/shaders/perlin_vertex.glsl", 0, "./data/shaders/gbuffer_fragment.glsl", &m_proceduralProgram);

	//Light Pre-Pass Shaders
	LoadShader("./data/shaders/composite_vertex.glsl", 0, "./data/shaders/directional_light_fragment.glsl", &m_dirLightProgram);
	LoadShader("./data/shaders/point_light_vertex.glsl", 0, "./data/shaders/point_light_fragment.glsl", &m_pointLightProgram);

	//Final Render Shader
	LoadShader("./data/shaders/composite_vertex.glsl", 0, "./data/shaders/composite_fragment.glsl", &m_compositeProgram);

	//Add some models.
	BuildQuad();
	BuildCube();
	BuildFrameBuffers();

	//AntTweakBar GUI Settings Initialisation
	TwBar* m_generalBar = TwNewBar("Debugging");
	TwAddVarRW(m_generalBar, "No Z-Buffer", TW_TYPE_BOOL8, &m_debug[0], "group=Gizmos");
	TwAddVarRW(m_generalBar, "Draw Grid", TW_TYPE_BOOL8, &m_debug[1], "group=Gizmos");
	TwAddVarRW(m_generalBar, "Draw PhysX", TW_TYPE_BOOL8, &m_debug[2], "group=Gizmos");
	TwAddVarRW(m_generalBar, "Draw Particles", TW_TYPE_BOOL8, &m_debug[3], "group=Gizmos");
	TwAddVarRW(m_generalBar, "Draw Point Lights", TW_TYPE_BOOL8, &m_debug[4], "group=Gizmos");
	TwAddVarRW(m_generalBar, "Draw Directional Lights", TW_TYPE_BOOL8, &m_debug[5], "group=Gizmos");

	TwBar* m_modelsBar = TwNewBar("Models");
	//AddFBXModel(new FBXModel("./data/models/characters/Pyro/pyro.fbx", 0.3f, 2.0f));

	//modTransform for bombs.
	mat4 bombTransform = glm::translate(vec3(0.1f, 0.4f, 0)) * glm::scale(vec3(0.1f)) * glm::rotate(glm::radians(-90.0f), vec3(1, 0, 0));
	//AddFBXModel("./data/models/items/bomb.fbx", vec3(5), 0.3f, 2.0f, );
	//AddFBXModel(new FBXModel("./data/models/characters/SoulSpear/soulspear.fbx", 0.3f, 2.0f));
	//m_physScene.AttachRigidBodyConvex(PxTransform(0, 10, 0), m_physScene.m_physics->createMaterial(1, 1, .2f), m_FBXModels.back(), 100.f, 100.f);

	//AddFBXModel(new FBXModel("./data/models/items/bomb.fbx", 0.3f, 2.0f, bombTransform));
	//m_physScene.AddRigidBodyDynamic(PxTransform(0, 10, 0), &PxSphereGeometry(1.5f), m_physScene.m_physics->createMaterial(0.0f, 0.0f, 0.1f), m_FBXModels.back(), 100.f);

	//AddFBXModel(new FBXModel("./data/models/characters/SoulSpear/soulspear.fbx", 0.3f, 2.0f));
	//m_physScene.AddRigidBodyDynamic(PxTransform(0, 7, 0), &PxCapsuleGeometry(0.25f, 4.0f), m_physScene.m_physics->createMaterial(1, 1, .2f), m_FBXModels.back(), 400.f);

	//AddPhysModel(new PhysModel("./data/models/items/bomb.fbx", vec3(0.0f, 10.0f, 0.0f), nullptr, m_physScene.m_physics->createMaterial(0.0f, 0.0f, 0.1f), &m_physScene, 100.0f, 0.3f, 2.0f, 3.2f, bombTransform));
	//AddPhysModel(new PhysModel("./data/models/items/bomb.fbx", vec3(0.1f, 16.0f, 0.0f), nullptr, m_physScene.m_physics->createMaterial(0.0f, 0.0f, 0.1f), &m_physScene, 100.0f, 0.3f, 2.0f, 3.2f, bombTransform));

	//AddPhysModel(new PhysModel("./data/models/items/bomb.fbx", vec3(0.1f, 23.0f, 0.0f), &PxSphereGeometry(1.5f), m_physScene.m_physics->createMaterial(0.0f, 0.0f, 0.1f), &m_physScene, 100.0f, 0.3f, 2.0f, 3.2f, bombTransform));
	////AddPhysModel(new PhysModel("./data/models/tank/battle_tank.fbx", vec3(0, 10, 0), &PxBoxGeometry(PxVec3(3)), m_physScene.m_physics->createMaterial(1, 1, .2f), &m_physScene, 400.0f, 0.3f, 2.0f, 100.0f));
	//AddPhysModel(new PhysModel("./data/models/characters/SoulSpear/soulspear.fbx", vec3(0, 7, 0), &PxCapsuleGeometry(0.25f, 4.0f), m_physScene.m_physics->createMaterial(1, 1, .2f), &m_physScene, 400.0f, 0.3f, 2.0f, 100.0f));
	//AddPhysModel(new PhysModel("./data/models/characters/SoulSpear/soulspear.fbx", vec3(0, 5, 0), &PxCapsuleGeometry(0.25f, 4.0f), m_physScene.m_physics->createMaterial(1, 1, .2f), &m_physScene, 400.0f, 0.3f, 2.0f, 100.0f));

	//Create Particle GUI bar and add particle emitters.
	TwBar* m_particlesBar = TwNewBar("Particles");
	AddParticleEmitter(new GPUEmitter(vec3(0), vec3(1), 50, 1.0f, 2.0f, 1.0f, 2.0f, 1.0f, 0.5f, 1.0f, 0.5f, vec4(1, 0.5, 0.5, 1), vec4(1, 0, 0, 1), EMIT_POINT, PMOVE_LINEAR, "./data/textures/particles/glow.png"));
	AddParticleEmitter(new GPUEmitter(vec3(0, 10, 0), vec3(30), 20, 3.0f, 5.5f, 1.0f, 2.0f, 1.0f, 0.75f, 1.0f, 1.0f, vec4(1, 1, 0.5, 1), vec4(0.65, 0.65, 0, 1), EMIT_RECTANGLE, PMOVE_WAVE, "./data/textures/particles/glow.png"));

	TwBar* m_lightingBar = TwNewBar("Lighting"); //Lighting window. Allows modification of lighting data.

	//Buttons to add and remove point lights from the program
	TwAddButton(m_lightingBar, "Add Point Light", [](void*){m_app->AddPointLight(); }, nullptr, "");
	TwAddButton(m_lightingBar, "Remove Point Light", [](void*){
		TwBar* m_lightingBar = TwGetBarByName("Lighting");
		if (m_lightingBar == nullptr || m_app->m_pointLights.size() <= 0) return;

		std::string prefix("P" + std::to_string(m_app->m_pointLights.size()) + "_");

		TwRemoveVar(m_lightingBar, std::string(prefix + "X").c_str());
		TwRemoveVar(m_lightingBar, std::string(prefix + "Y").c_str());
		TwRemoveVar(m_lightingBar, std::string(prefix + "Z").c_str());
		TwRemoveVar(m_lightingBar, std::string(prefix + "Color").c_str());
		TwRemoveVar(m_lightingBar, std::string(prefix + "Radius").c_str());

		m_app->m_pointLights.pop_back();
	}, nullptr, "");

	//Buttons to add and remove directional lights from the program
	TwAddButton(m_lightingBar, "Add Directional Light", [](void*){m_app->AddDirectionalLight(); }, nullptr, "");
	TwAddButton(m_lightingBar, "Remove Directional Light", [](void*){
		TwBar* m_lightingBar = TwGetBarByName("Lighting");
		if (m_lightingBar == nullptr || m_app->m_dirLights.size() <= 0) return;

		std::string prefix("D" + std::to_string(m_app->m_dirLights.size()) + "_");

		TwRemoveVar(m_lightingBar, std::string(prefix + "Direction").c_str());
		TwRemoveVar(m_lightingBar, std::string(prefix + "Color").c_str());

		m_app->m_dirLights.pop_back();
	}, nullptr, "");

	TwAddVarRW(m_lightingBar, "Ambient Color", TW_TYPE_COLOR3F, &m_ambCol, "");
	AddPointLight(vec3(0, 20, 10), vec3(1), 100);
	AddDirectionalLight(vec3(0, -1, 0), vec3(1)); 
	
	//Initialise Procedural Landscape
	vec2 meshDims = glm::vec2(800, 800);
	glm::ivec2 textDims = glm::ivec2(256,256);

	BuildProceduralGrid(meshDims, textDims);
	BuildPerlinTexture(textDims, m_pOct, m_pAmp, m_pPers);
	m_physScene.AddHeightMap(m_perlinData, m_physScene.m_physics->createMaterial(1, 1, 1), textDims, vec3(meshDims.x, m_pScale, meshDims.y));

	//Add procedural generation variables to debugging GUI window. 
	TwAddVarRW(m_generalBar, "Scale",		TW_TYPE_FLOAT, &m_pScale,	"group=ProceduralGeneration min=0 step=0.05");
	TwAddVarRW(m_generalBar, "Octaves",		TW_TYPE_INT32, &m_pOct,		"group=ProceduralGeneration min=1 max=10"); //Don't want octaves to go too high. Will choke CPU on reload.
	TwAddVarRW(m_generalBar, "Amplitude",	TW_TYPE_FLOAT, &m_pAmp,		"group=ProceduralGeneration min=0 step=0.01");
	TwAddVarRW(m_generalBar, "Persistance", TW_TYPE_FLOAT, &m_pPers,	"group=ProceduralGeneration min=0 step=0.01");
	//Only the scale variable affects simulation in realtime. All others require recreating the perlinTexture.

	m_cloths.push_back(new ClothData(1, 10, 20));
	AddCloth(m_physScene.AddCloth(vec3(0, 40, 0), m_cloths.back()->m_vertexCount, m_cloths.back()->m_indexCount, m_cloths.back()->m_vertices, m_cloths.back()->m_indices));
	m_cloths.back()->GLGenBuffers();

	Gizmos::create();
	return true;
}

bool VirtualWorld::shutdown(){
	//Game Asset Termination
	//Delete all particles and their texture memory
	for (GPUEmitter* particle : m_particleEmitters) {
		delete particle;
	}
	//Delete all models and their texture memory
	for (FBXModel* model : m_FBXModels) {
		delete model;
	}


	for (ClothData* cloth : m_cloths) {
		delete cloth;
	}

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

	//GUI Termination
	TwTerminate(); //End ATB
	Gizmos::destroy(); //End Gizmos

	return Application::shutdown();
}


bool VirtualWorld::update(){
	if (!Application::update()){
		return false;
	}
	m_oCamera.update(m_fDeltaTime);
	Gizmos::clear();

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

	m_physScene.Update(m_fDeltaTime, m_debug[2]);

	for (FBXModel* model : m_FBXModels) {
		model->Update(m_fDeltaTime);
	}


	//Parenting of particle position.
	//if (m_pointLights.size() > 0)
	//	m_particleEmitters[0]->m_pos = m_pointLights[0].m_pos;

	return true;
}


void VirtualWorld::draw(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Enable Depth Culling.
	glEnable(GL_DEPTH_TEST);

	//Enable Face Culling of polygons facing away from camera.
	glEnable(GL_CULL_FACE);

	glBindFramebuffer(GL_FRAMEBUFFER, m_gBufferFBO);
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearBufferfv(GL_COLOR, 0, (float*)&vec4(0.0f));
	glClearBufferfv(GL_COLOR, 1, (float*)&vec4(0.0f));
	glClearBufferfv(GL_COLOR, 2, (float*)&vec4(0.5f));

	glDepthFunc(GL_LESS);
	//glDepthMask(GL_FALSE);
	glUseProgram(m_proceduralProgram);

	//int loc = glGetUniformLocation(m_proceduralProgram, "world");
	//glUniformMatrix4fv(loc, 1, GL_FALSE, (float*)&m_oCamera.getWorldTransform());

	int loc = glGetUniformLocation(m_proceduralProgram, "view");
	glUniformMatrix4fv(loc, 1, GL_FALSE, (float*)&m_oCamera.getView());

	loc = glGetUniformLocation(m_proceduralProgram, "projView");
	glUniformMatrix4fv(loc, 1, GL_FALSE, (float*)&m_oCamera.getProjectionView());

	loc = glGetUniformLocation(m_proceduralProgram, "scale");
	glUniform1f(loc, m_pScale);

	loc = glGetUniformLocation(m_proceduralProgram, "perlinTexture");
	glUniform1i(loc, 0);

	loc = glGetUniformLocation(m_proceduralProgram, "deferred");
	glUniform1i(loc, true);

	loc = glGetUniformLocation(m_proceduralProgram, "worldSize");
	glUniform2fv(loc, 1, (float*)&m_perlinWorldSize);

	loc = glGetUniformLocation(m_proceduralProgram, "textureSize");
	glUniform2fv(loc, 1, (float*)&m_perlinTextureSize);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_perlinTexture);

	glBindVertexArray(m_planeMesh.m_VAO);
	glDrawElements(GL_TRIANGLES, m_planeMesh.m_indexCount, GL_UNSIGNED_INT, 0);

	glUseProgram(m_gBufferProgram);

	loc = glGetUniformLocation(m_gBufferProgram, "view");
	glUniformMatrix4fv(loc, 1, GL_FALSE, (float*)&m_oCamera.getView());

	loc = glGetUniformLocation(m_gBufferProgram, "projView");
	glUniformMatrix4fv(loc, 1, GL_FALSE, (float*)&m_oCamera.getProjectionView());

	//Opaque Drawing

	for (ClothData* cloth : m_cloths) {
		cloth->draw();
	}
	for (FBXModel* model : m_FBXModels){
		model->RenderDeferred(m_oCamera);
	}

	//Light Rendering
	glBindFramebuffer(GL_FRAMEBUFFER, m_lightFBO);
	glEnable(GL_BLEND);//Enable transparency blending
	glClear(GL_COLOR_BUFFER_BIT);

	
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
	glDepthMask(GL_FALSE); //Disable newly drawn objects writing to the depth buffer. Allows all FX to be drawn without culling.
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glUseProgram(m_gBufferProgram);
	for (unsigned int i = 0; i < m_particleEmitters.size(); i++) {
		m_particleEmitters[i]->Render(m_fCurrTime, m_oCamera, true);
		if (m_debug[3]) {
			m_particleEmitters[i]->RenderGizmos();
		}
	}

	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND); //No more transparent rendering, turn off the blend.
	glBlendFunc(GL_ONE, GL_ONE);

	//Draw Gizmos at the end of the transparency pass, to retain depth culling without lighting.
	if (!m_debug[0]) {
		Gizmos::draw(m_oCamera.getProjectionView());
	}

	//Composite Rendering
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(0.3f, 0.3f, 0.3f, 1);

	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(m_compositeProgram);

	glUniform1i(glGetUniformLocation(m_compositeProgram, "albedoTexture"), 0);
	glUniform1i(glGetUniformLocation(m_compositeProgram, "lightTexture"), 1);
	glUniform1i(glGetUniformLocation(m_compositeProgram, "fxTexture"), 2);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_albedoTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_lightTexture);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_fxTexture);

	loc = glGetUniformLocation(m_compositeProgram, "ambCol");
	glUniform3fv(loc, 1, (float*)&m_ambCol);

	glDepthFunc(GL_ALWAYS);//Interface data / The screenspace quad should always be drawn.

	//Render the screen texture to the screen quad.
	glBindVertexArray(m_screenspaceQuad.m_VAO);
	glDrawElements(GL_TRIANGLES, m_screenspaceQuad.m_indexCount, GL_UNSIGNED_INT, 0);

	//GUI and Interface Drawing.

	//Gizmos without any depth culling.
	if (m_debug[0]) {
		Gizmos::draw(m_oCamera.getProjectionView());
	}

	TwDraw();

	Application::draw();
}

void VirtualWorld::RenderDirectionalLights() {
	glUseProgram(m_dirLightProgram);

	//Tell the shader which slot holds each texture
	glUniform1i(glGetUniformLocation(m_dirLightProgram, "positionTexture"), 0);
	glUniform1i(glGetUniformLocation(m_dirLightProgram, "normalTexture"),   1);
	glUniform1i(glGetUniformLocation(m_dirLightProgram, "specularTexture"), 2);

	//Bind the textures to the slots
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_positionTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_normalTexture);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_specularTexture);

	//Assign uniform locations outside of loop, for efficiency.
	int lightDir_uniform = glGetUniformLocation(m_dirLightProgram, "lightDir");
	int lightCol_uniform = glGetUniformLocation(m_dirLightProgram, "lightCol");

	for (DirectionalLight light : m_dirLights) {
		vec4 viewspaceLightDir = m_oCamera.getView() * vec4(glm::normalize(light.m_dir), 0);
		if (m_debug[5]) {
			Gizmos::addSphere(-light.m_dir * 15000, 350, 8, 8, vec4(light.m_color, 0.5));
		}

		glUniform3fv(lightDir_uniform, 1, (float*)&viewspaceLightDir);
		glUniform3fv(lightCol_uniform, 1, (float*)&light.m_color);

		glBindVertexArray(m_screenspaceQuad.m_VAO);
		glDrawElements(GL_TRIANGLES, m_screenspaceQuad.m_indexCount, GL_UNSIGNED_INT, 0);
	}
}

void VirtualWorld::RenderPointLights() {
	glUseProgram(m_pointLightProgram);

	int loc = glGetUniformLocation(m_pointLightProgram, "projView");
	glUniformMatrix4fv(loc, 1, GL_FALSE, (float*)&m_oCamera.getProjectionView());

	glUniform1i(glGetUniformLocation(m_pointLightProgram, "positionTexture"), 0);
	glUniform1i(glGetUniformLocation(m_pointLightProgram, "normalTexture"),   1);
	glUniform1i(glGetUniformLocation(m_pointLightProgram, "specularTexture"), 2);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_positionTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_normalTexture);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_specularTexture);

	//Assign uniform locations outside of loop, for efficiency.
	int lightPos_uniform = glGetUniformLocation(m_pointLightProgram, "lightPos");
	int lightViewPos_uniform = glGetUniformLocation(m_pointLightProgram, "lightViewPos");
	int lightRadius_uniform = glGetUniformLocation(m_pointLightProgram, "lightRadius");
	int lightCol_uniform = glGetUniformLocation(m_pointLightProgram, "lightCol");


	for (PointLight light : m_pointLights) {
		vec4 viewspaceLightPos = m_oCamera.getView() * vec4(light.m_pos, 1);
		if (m_debug[4]) {
			Gizmos::addAABB(light.m_pos, vec3(0.5), vec4(light.m_color, 1));
		}

		glUniform3fv(lightPos_uniform, 1, (float*)&light.m_pos);
		glUniform3fv(lightViewPos_uniform, 1, (float*)&viewspaceLightPos);
		glUniform1f(lightRadius_uniform, light.m_radius);
		glUniform3fv(lightCol_uniform, 1, (float*)&light.m_color);

		glBindVertexArray(m_lightCube.m_VAO);
		glDrawElements(GL_TRIANGLES, m_lightCube.m_indexCount, GL_UNSIGNED_INT, 0);
	}
}

//Convenience function to initialise directional light data within simulation and add it to GUI.
void VirtualWorld::AddDirectionalLight(vec3 a_dir, vec3 a_color) {
	m_dirLights.push_back(DirectionalLight(a_dir, a_color));

	TwBar* m_lightingBar = TwGetBarByName("Lighting");

	if (m_lightingBar == nullptr) return; //No Lighting bar was found.

	//Formatting the light data with a unique prefix, so AntTweakBar can display it. ATB rejects duplicate keyvalue names.
	std::string prefix("DL" + std::to_string(m_dirLights.size()) + "_");
	std::string group = std::string("group=DirectionalLight" + std::to_string(m_dirLights.size()));

	TwAddVarRW(m_lightingBar, std::string(prefix + "Direction").c_str(), TW_TYPE_DIR3F, &m_dirLights.back().m_dir, group.c_str());
	TwAddVarRW(m_lightingBar, std::string(prefix + "Color").c_str(), TW_TYPE_COLOR3F, &m_dirLights.back().m_color, group.c_str());

}

//Convenience function that creates point light data and adds it to the lighting GUI window.
void VirtualWorld::AddPointLight(vec3 a_pos, vec3 a_color, float a_radius) {
	m_pointLights.push_back(PointLight(a_pos, a_color, a_radius));

	TwBar* m_lightingBar = TwGetBarByName("Lighting");

	if (m_lightingBar == nullptr) return; //No Lighting bar was found.

	std::string prefix("PL" + std::to_string(m_pointLights.size()) + "_");
	std::string group = std::string("group=PointLight" + std::to_string(m_pointLights.size()));

	TwAddVarRW(m_lightingBar, std::string(prefix + "X").c_str(), TW_TYPE_FLOAT, &m_pointLights.back().m_pos.x, (std::string("step=0.1 ") + group).c_str());
	TwAddVarRW(m_lightingBar, std::string(prefix + "Y").c_str(), TW_TYPE_FLOAT, &m_pointLights.back().m_pos.y, (std::string("step=0.1 ") + group).c_str());
	TwAddVarRW(m_lightingBar, std::string(prefix + "Z").c_str(), TW_TYPE_FLOAT, &m_pointLights.back().m_pos.z, (std::string("step=0.1 ") + group).c_str());
	TwAddVarRW(m_lightingBar, std::string(prefix + "Color").c_str(), TW_TYPE_COLOR3F, &m_pointLights.back().m_color, group.c_str());
	TwAddVarRW(m_lightingBar, std::string(prefix + "Radius").c_str(), TW_TYPE_FLOAT, &m_pointLights.back().m_radius, (std::string("min=0 ") + group).c_str());
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
	//m_cloths.push_back(a_cloth);
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
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA16F, m_iWidth, m_iHeight);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glGenTextures(1, &m_normalTexture);
	glBindTexture(GL_TEXTURE_2D, m_normalTexture);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA16F, m_iWidth, m_iHeight);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glGenTextures(1, &m_specularTexture);
	glBindTexture(GL_TEXTURE_2D, m_specularTexture);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA16F, m_iWidth, m_iHeight);
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

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, m_fxTexture, 0);
	//Pass the depth buffer from the geometry pass into the FX framebuffer, so depth culling is done correctly.
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthTexture);

	GLenum fxTargets[] = { GL_COLOR_ATTACHMENT3 };
	glDrawBuffers(1, fxTargets);

	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
		printf("Error creating light framebuffer!\n");

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void VirtualWorld::BuildProceduralGrid(vec2 a_realDims, glm::ivec2 a_dims){
	m_perlinWorldSize = a_realDims;
	m_perlinTextureSize = a_dims;

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

	m_perlinData = new float[a_dims.x * a_dims.y];

	for (int y = 0; y < a_dims.y; y++) {
		for (int x = 0; x < a_dims.x; x++) {

			float amplitude = a_amplitude;
			float freq = 1;

			m_perlinData[y*a_dims.x + x] = 0;

			for (int o = 0; o < a_octaves; o++) {
				float perlinSample = glm::perlin(vec2((float)x, (float)y) * scale * freq) * 0.5f + 0.5f;

				perlinSample *= amplitude;
				m_perlinData[y*a_dims.x + x] += perlinSample;

				amplitude *= a_persistance;
				freq *= 2;
			}
		}
	}

	glGenTextures(1, &m_perlinTexture);
	glBindTexture(GL_TEXTURE_2D, m_perlinTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, a_dims.x, a_dims.y, 0, GL_RED, GL_FLOAT, m_perlinData);

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

	LoadShader("./data/shaders/gbuffer_vertex.glsl", 0, "./data/shaders/gbuffer_fragment.glsl", &m_gBufferProgram);
	LoadShader("./data/shaders/perlin_vertex.glsl", 0, "./data/shaders/gbuffer_fragment.glsl", &m_proceduralProgram);
	LoadShader("./data/shaders/composite_vertex.glsl", 0, "./data/shaders/composite_fragment.glsl", &m_compositeProgram);
	LoadShader("./data/shaders/composite_vertex.glsl", 0, "./data/shaders/directional_light_fragment.glsl", &m_dirLightProgram);
	LoadShader("./data/shaders/point_light_vertex.glsl", 0, "./data/shaders/point_light_fragment.glsl", &m_pointLightProgram);

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