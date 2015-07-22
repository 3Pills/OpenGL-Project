#include "VirtualWorld.h"
#include "AntTweakBar.h"
#include <string>

VirtualWorld::VirtualWorld(): m_oCamera(50), m_pScale(1.f), m_pOct(6), m_pAmp(1.f), m_pPers(0.3f){
	Application::Application();
}
VirtualWorld::~VirtualWorld(){}

bool VirtualWorld::startup(){
	if (!Application::startup()){
		return false;
	}

	//AntTweakBar Initialisation.
	TwInit(TW_OPENGL_CORE, nullptr);
	TwWindowSize(m_iWidth, m_iHeight);

	//Binding input callbacks for ATB GUI.
	glfwSetMouseButtonCallback(m_window, OnMouseButton);
	glfwSetCursorPosCallback(m_window, OnMousePosition);
	glfwSetScrollCallback(m_window, OnMouseScroll);
	glfwSetKeyCallback(m_window, OnKey);
	glfwSetCharCallback(m_window, OnChar);
	glfwSetWindowSizeCallback(m_window, OnWindowResize);

	//Initialise camera
	m_oCamera.setPerspective(glm::radians(50.0f), 1280.0f / 720.0f, 0.1f, 20000.0f);

	//Shader Initialisation
	//Opaque Geometry
	LoadShader("./data/shaders/gbuffer_vertex.glsl", 0, "./data/shaders/gbuffer_fragment.glsl", &m_gBufferProgram);
	LoadShader("./data/shaders/perlin_vertex.glsl", 0, "./data/shaders/gbuffer_fragment.glsl", &m_proceduralProgram);

	//Light Pre-Pass
	LoadShader("./data/shaders/composite_vertex.glsl", 0, "./data/shaders/directional_light_fragment.glsl", &m_dirLightProgram);
	LoadShader("./data/shaders/point_light_vertex.glsl", 0, "./data/shaders/point_light_fragment.glsl", &m_pointLightProgram);

	//Transparency
	//LoadShader("./data/shaders/composite_vertex.glsl", 0, "./data/shaders/directional_light_fragment.glsl", &m_dirLightProgram);

	//Final Render
	LoadShader("./data/shaders/composite_vertex.glsl", 0, "./data/shaders/composite_fragment.glsl", &m_compositeProgram);


	//Add some models.
	BuildQuad();
	BuildCube();
	BuildFrameBuffers();

	//AntTweakBar GUI Settings Initialisation
	TwBar* m_generalBar = TwNewBar("Debugging");
	TwAddVarRW(m_generalBar, "Ignore Depth", TW_TYPE_BOOL8, &m_debug[0], "group=Gizmos");
	TwAddVarRW(m_generalBar, "Render Grid", TW_TYPE_BOOL8, &m_debug[1], "group=Gizmos");

	TwBar* m_modelsBar = TwNewBar("Models");
	AddFBXModel("./data/models/characters/SoulSpear/soulspear.fbx", 1, 2);
	AddFBXModel("./data/models/characters/Pyro/pyro.fbx", 1, 2);

	//Create Particle GUI bar and add particle emitters.
	TwBar* m_particlesBar = TwNewBar("Particles");
	AddParticleEmitter(vec3(0), vec3(1), 50, 1.0f, 2.0f, 1.0f, 2.0f, 1.0f, 0.5f, 1.0f, 0.5f, vec4(1, 0.5, 0.5, 1), vec4(1, 0, 0, 1), EMIT_POINT, PMOVE_LINEAR, "./data/textures/particles/glow.png");
	AddParticleEmitter(vec3(0, 10, 0), vec3(30), 20, 3.0f, 5.5f, 1.0f, 2.0f, 1.0f, 0.75f, 1.0f, 1.0f, vec4(1, 1, 0.5, 1), vec4(0.65, 0.65, 0, 1), EMIT_RECTANGLE, PMOVE_WAVE, "./data/textures/particles/glow.png");

	TwBar* m_lightingBar = TwNewBar("Lighting"); //Lighting window. Allows modification of lighting data.
	AddPointLight(vec3(0, 20, 10), vec3(1), 100); //Add point light data. Creates the data and puts it into the GUI.
	
	//Initialise Procedural Landscape
	glm::ivec2 dims = glm::ivec2(256, 256);
	BuildProceduralGrid(vec2(40, 40), dims);
	BuildPerlinTexture(glm::ivec2(256, 256), m_pOct, m_pAmp, m_pPers);

	//Add procedural generation variables to debugging GUI window. 
	TwAddVarRW(m_generalBar, "Scale",		TW_TYPE_FLOAT, &m_pScale,	"group=ProceduralGeneration min=0 step=0.05");
	TwAddVarRW(m_generalBar, "Octaves",		TW_TYPE_INT32, &m_pOct,		"group=ProceduralGeneration min=1 max=10"); //Don't want octaves to go too high. Will choke CPU on reload.
	TwAddVarRW(m_generalBar, "Amplitude",	TW_TYPE_FLOAT, &m_pAmp,		"group=ProceduralGeneration min=0 step=0.01");
	TwAddVarRW(m_generalBar, "Persistance", TW_TYPE_FLOAT, &m_pPers,	"group=ProceduralGeneration min=0 step=0.01");
	//Only the scale variable affects simulation in realtime. All others require recreating the perlinTexture.

	Gizmos::create();
	return true;
}

//Convenience function that creates point light data and adds it to the lighting GUI window.
void VirtualWorld::AddPointLight(vec3 a_pos, vec3 a_color, float a_radius) {
	m_pointLights.push_back(new PointLight(a_pos, a_color, a_radius));

	TwBar* m_lightingBar = TwGetBarByName("Lighting");

	if (m_lightingBar == nullptr) return; //No Lighting bar was found.

	std::string prefix(std::to_string(m_pointLights.size()) + "_");
	std::string group = std::string("group=PointLight" + std::to_string(m_pointLights.size()));

	TwAddVarRW(m_lightingBar, std::string(prefix + "X").c_str(),		TW_TYPE_FLOAT,		&m_pointLights.back()->m_pos.x,		group.c_str());
	TwAddVarRW(m_lightingBar, std::string(prefix + "Y").c_str(),		TW_TYPE_FLOAT,		&m_pointLights.back()->m_pos.y,		group.c_str());
	TwAddVarRW(m_lightingBar, std::string(prefix + "Z").c_str(),		TW_TYPE_FLOAT,		&m_pointLights.back()->m_pos.z,		group.c_str());
	TwAddVarRW(m_lightingBar, std::string(prefix + "Color").c_str(),	TW_TYPE_COLOR3F,	&m_pointLights.back()->m_color,		group.c_str());
	TwAddVarRW(m_lightingBar, std::string(prefix + "Radius").c_str(),	TW_TYPE_FLOAT,		&m_pointLights.back()->m_radius,	(std::string("min=0 ") + group).c_str());
}

//Convenience function that add an FBXModel to the vector array, while also adding it to the Model GUI window.
void VirtualWorld::AddFBXModel(const char* a_filename, float a_roughness, float a_fresnelScale) {
	m_FBXModels.push_back(new FBXModel(a_filename));
	m_FBXModels.back()->m_roughness = a_roughness;
	m_FBXModels.back()->m_fresnelScale = a_fresnelScale;

	TwBar* m_modelBar = TwGetBarByName("Models");

	if (m_modelBar == nullptr) return; //No Models bar was found.

	std::string prefix(std::to_string(m_FBXModels.size()) + "_");
	std::string group = std::string("group=Model" + std::to_string(m_FBXModels.size()));

	TwAddVarRW(m_modelBar, std::string(prefix + "Roughness").c_str(),		TW_TYPE_FLOAT, &m_FBXModels.back()->m_roughness,	(std::string("min=0 step=0.05 max=100.0 ") + group).c_str());
	TwAddVarRW(m_modelBar, std::string(prefix + "Fresnel_Scale").c_str(),	TW_TYPE_FLOAT, &m_FBXModels.back()->m_fresnelScale, (std::string("min=0 step=0.05 max=100.0 ") + group).c_str());
}

void VirtualWorld::AddParticleEmitter(vec3 a_pos, vec3 a_extents, unsigned int a_maxParticles,	float a_lifespanMin, float a_lifespanMax, float a_velocityMin, float a_velocityMax, 
	float a_fadeIn, float a_fadeOut, float a_startSize, float a_endSize, vec4 a_startColor, vec4 a_endColor, EmitType a_emitType, MoveType a_moveType, char* a_szFilename) {
	m_particleEmitters.push_back(new GPUEmitter);
	m_particleEmitters.back()->Init(a_pos, a_extents, a_maxParticles, a_lifespanMin, a_lifespanMax, a_velocityMin, a_velocityMax, a_fadeIn, a_fadeOut, a_startSize, a_endSize, a_startColor, a_endColor, a_emitType, a_moveType, a_szFilename);

	TwBar* m_particlesBar = TwGetBarByName("Particles");

	if (m_particlesBar == nullptr) return; //No Particles bar was found.

	std::string prefix(std::to_string(m_particleEmitters.size()) + "_");
	std::string group = std::string("group=ParticleEmitter" + std::to_string(m_particleEmitters.size()));

	TwEnumVal emitTypes[] = { { EMIT_POINT, "Point" }, { EMIT_LINE, "Line" }, { EMIT_PLANE, "Plane" }, { EMIT_RING, "Ring" }, { EMIT_OUTER_RING, "Outer Ring" },
	{ EMIT_RECTANGLE, "Rectangle" }, { EMIT_OUTER_RECTANGLE, "Outer Rectangle" }, { EMIT_SPHERE, "Sphere" }, { EMIT_OUTER_SPHERE, "Outer Sphere" } };
	TwType emitType = TwDefineEnum("EmitType", emitTypes, 9);

	TwEnumVal moveTypes[] = { { PMOVE_LINEAR, "Linear" }, { PMOVE_WAVE, "Wave" } };
	TwType moveType = TwDefineEnum("MoveType", moveTypes, 2);

	TwAddVarRW(m_particlesBar, std::string(prefix + "Position_X").c_str(),		TW_TYPE_FLOAT,		&m_particleEmitters.back()->m_pos.x,		(std::string("step=0.1 min=0 ") + group).c_str());
	TwAddVarRW(m_particlesBar, std::string(prefix + "Position_Y").c_str(),		TW_TYPE_FLOAT,		&m_particleEmitters.back()->m_pos.y,		(std::string("step=0.1 min=0 ") + group).c_str());
	TwAddVarRW(m_particlesBar, std::string(prefix + "Position_Z").c_str(),		TW_TYPE_FLOAT,		&m_particleEmitters.back()->m_pos.z,		(std::string("step=0.1 min=0 ") + group).c_str());
	TwAddVarRW(m_particlesBar, std::string(prefix + "Extents_X").c_str(),		TW_TYPE_FLOAT,		&m_particleEmitters.back()->m_extents.x,	(std::string("step=0.1 min=0 ") + group).c_str());
	TwAddVarRW(m_particlesBar, std::string(prefix + "Extents_Y").c_str(),		TW_TYPE_FLOAT,		&m_particleEmitters.back()->m_extents.y,	(std::string("step=0.1 min=0 ") + group).c_str());
	TwAddVarRW(m_particlesBar, std::string(prefix + "Extents_Z").c_str(),		TW_TYPE_FLOAT,		&m_particleEmitters.back()->m_extents.z,	(std::string("step=0.1 min=0 ") + group).c_str());
	TwAddVarRW(m_particlesBar, std::string(prefix + "Emit_Type").c_str(),		emitType,			&m_particleEmitters.back()->m_emitType,		group.c_str());
	TwAddVarRW(m_particlesBar, std::string(prefix + "Move_Type").c_str(),		moveType,			&m_particleEmitters.back()->m_moveType,		group.c_str());
	TwAddVarRW(m_particlesBar, std::string(prefix + "Lifespan_Max").c_str(), 	TW_TYPE_FLOAT,		&m_particleEmitters.back()->m_lifespanMax,	(std::string("step=0.05 min=0 ") + group).c_str());
	TwAddVarRW(m_particlesBar, std::string(prefix + "Lifespan_Min").c_str(), 	TW_TYPE_FLOAT,		&m_particleEmitters.back()->m_lifespanMin,	(std::string("step=0.05 min=0 ") + group).c_str());
	TwAddVarRW(m_particlesBar, std::string(prefix + "Velocity_Min").c_str(), 	TW_TYPE_FLOAT,		&m_particleEmitters.back()->m_velocityMin,	(std::string("step=0.05 min=0 ") + group).c_str());
	TwAddVarRW(m_particlesBar, std::string(prefix + "Velocity_Max").c_str(), 	TW_TYPE_FLOAT,		&m_particleEmitters.back()->m_velocityMax,	(std::string("step=0.05 min=0 ") + group).c_str());
	TwAddVarRW(m_particlesBar, std::string(prefix + "Fade_In").c_str(),			TW_TYPE_FLOAT,		&m_particleEmitters.back()->m_fadeIn,		(std::string("step=0.05 min=0 ") + group).c_str());
	TwAddVarRW(m_particlesBar, std::string(prefix + "Fade_Out").c_str(),		TW_TYPE_FLOAT,		&m_particleEmitters.back()->m_fadeOut,		(std::string("step=0.05 min=0 ") + group).c_str());
	TwAddVarRW(m_particlesBar, std::string(prefix + "Start_Size").c_str(),		TW_TYPE_FLOAT,		&m_particleEmitters.back()->m_startSize,	(std::string("step=0.05 min=0 ") + group).c_str());
	TwAddVarRW(m_particlesBar, std::string(prefix + "End_Size").c_str(),		TW_TYPE_FLOAT,		&m_particleEmitters.back()->m_endSize,		(std::string("step=0.05 min=0 ") + group).c_str());
	TwAddVarRW(m_particlesBar, std::string(prefix + "Start_Color").c_str(),		TW_TYPE_COLOR4F,	&m_particleEmitters.back()->m_startColor,	group.c_str());
	TwAddVarRW(m_particlesBar, std::string(prefix + "End_Color").c_str(),		TW_TYPE_COLOR4F,	&m_particleEmitters.back()->m_endColor,		group.c_str());
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

	for (PointLight* light : m_pointLights) {
		delete light;
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

	for (FBXModel* model : m_FBXModels) {
		model->Update(m_fCurrTime);
	}

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

	//Parenting of particle position.
	m_particleEmitters[0]->m_pos = m_pointLights[0]->m_pos;

	return true;
}


void VirtualWorld::draw(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Enable Depth Culling.
	glEnable(GL_DEPTH_TEST);

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
	
	for (FBXModel* model : m_FBXModels){
		model->RenderDeferred(m_oCamera);
	}

	//Light Rendering
	glBindFramebuffer(GL_FRAMEBUFFER, m_lightFBO);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(m_dirLightProgram);
	
	loc = glGetUniformLocation(m_dirLightProgram, "positionTexture");
	glUniform1i(loc, 0);
	loc = glGetUniformLocation(m_dirLightProgram, "normalTexture");
	glUniform1i(loc, 1);
	loc = glGetUniformLocation(m_dirLightProgram, "specularTexture");
	glUniform1i(loc, 2);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_positionTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_normalTexture);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_specularTexture);
	
	//RenderDirectionalLight(vec3(1, 0, 0), vec3(1, 0, 0));
	//RenderDirectionalLight(vec3(0, 1, 0), vec3(0, 1, 0));
	//RenderDirectionalLight(vec3(0, 0, 1), vec3(0, 0, 1));
	//RenderDirectionalLight(vec3(0, -1, 0), vec3(1));
	//RenderDirectionalLight(vec3(0, 0, -1), vec3(0.7));

	glUseProgram(m_pointLightProgram);
	
	loc = glGetUniformLocation(m_pointLightProgram, "projView");
	glUniformMatrix4fv(loc, 1, GL_FALSE, (float*)&m_oCamera.getProjectionView());

	loc = glGetUniformLocation(m_pointLightProgram, "positionTexture");
	glUniform1i(loc, 0);
	loc = glGetUniformLocation(m_pointLightProgram, "normalTexture");
	glUniform1i(loc, 1);
	loc = glGetUniformLocation(m_pointLightProgram, "specularTexture");
	glUniform1i(loc, 2);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_positionTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_normalTexture);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_specularTexture);

	glCullFace(GL_FRONT);
	for (PointLight* light : m_pointLights) {
		Gizmos::addTransform(glm::translate(light->m_pos), 1);
		RenderPointLight(light->m_pos, light->m_radius, light->m_color);
	}
	glCullFace(GL_BACK);

	glDisable(GL_BLEND);

	//Effects Rendering
	glBindFramebuffer(GL_FRAMEBUFFER, m_fxFBO);
	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_BLEND);//Enable fx rendering
	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_FALSE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glUseProgram(m_gBufferProgram);
	for (int i = 0; i < m_particleEmitters.size(); i++) {
		m_particleEmitters[i]->Render(m_fCurrTime, m_oCamera, true);
	}

	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	//Draw Gizmos at the end of the transparency pass, to retain depth culling without lighting.
	if (!m_debug[0]) {
		Gizmos::draw(m_oCamera.getProjectionView());
	}

	//Composite Rendering
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(0.3, 0.3, 0.3, 1);

	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(m_compositeProgram);

	loc = glGetUniformLocation(m_compositeProgram, "albedoTexture");
	glUniform1i(loc, 0);
	loc = glGetUniformLocation(m_compositeProgram, "lightTexture");
	glUniform1i(loc, 1);
	loc = glGetUniformLocation(m_compositeProgram, "fxTexture");
	glUniform1i(loc, 2);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_albedoTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_lightTexture);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_fxTexture);

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

void VirtualWorld::RenderDirectionalLight(vec3 a_lightDir, vec3 a_lightColor) {
	vec4 viewspaceLightDir = m_oCamera.getView() * vec4(glm::normalize(a_lightDir), 0);

	int loc = glGetUniformLocation(m_dirLightProgram, "lightDir");
	glUniform3fv(loc, 1, (float*)&viewspaceLightDir);

	loc = glGetUniformLocation(m_dirLightProgram, "lightCol");
	glUniform3fv(loc, 1, (float*)&a_lightColor);

	glBindVertexArray(m_screenspaceQuad.m_VAO);
	glDrawElements(GL_TRIANGLES, m_screenspaceQuad.m_indexCount, GL_UNSIGNED_INT, 0);
}

void VirtualWorld::RenderPointLight(vec3 a_lightPos, float a_radius, vec3 a_lightColor) {
	vec4 viewspaceLightPos = m_oCamera.getView() * vec4(a_lightPos, 1);

	int loc = glGetUniformLocation(m_pointLightProgram, "lightPos");
	glUniform3fv(loc, 1, &a_lightPos[0]);

	loc = glGetUniformLocation(m_pointLightProgram, "lightViewPos");
	glUniform3fv(loc, 1, &viewspaceLightPos[0]);

	loc = glGetUniformLocation(m_pointLightProgram, "lightRadius");
	glUniform1f(loc, a_radius);

	loc = glGetUniformLocation(m_pointLightProgram, "lightCol");
	glUniform3fv(loc, 1, &a_lightColor[0]);

	glBindVertexArray(m_lightCube.m_VAO);
	glDrawElements(GL_TRIANGLES, m_lightCube.m_indexCount, GL_UNSIGNED_INT, 0);
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
				float perlinSample = glm::perlin(vec2((float)x, (float)y) * scale * freq) * 0.5 + 0.5;

				perlinSample *= amplitude;
				m_perlinData[y*a_dims.x + x] += perlinSample;

				amplitude *= a_persistance;
				freq *= 2;
			}
		}
	}

	glGenTextures(1, &m_perlinTexture);
	glBindTexture(GL_TEXTURE_2D, m_perlinTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, a_dims.x, a_dims.y, 0, GL_RED, GL_FLOAT, m_perlinData);

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