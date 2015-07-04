#include "Animation.h"
#include "Utility.h"

Animation::Animation() : m_oCamera(50), m_vAmbCol(vec3(0.4f)), m_vLightCol(vec3(0.85f)), m_vLightPos(vec3(0, 10, 0)), m_fSpecPow(16){
	Application::Application();
}
Animation::~Animation(){}

bool Animation::startup(){
	if (!Application::startup()){
		return false;
	}
	glEnable(GL_DEPTH_TEST);

	TwInit(TW_OPENGL_CORE, nullptr);
	TwWindowSize(1280, 720);

	glfwSetMouseButtonCallback(m_window, OnMouseButton);
	glfwSetCursorPosCallback(m_window, OnMousePosition);
	glfwSetScrollCallback(m_window, OnMouseScroll);

	glfwSetKeyCallback(m_window, OnKey);
	glfwSetCharCallback(m_window, OnChar);

	glfwSetWindowSizeCallback(m_window, OnWindowResize);

	m_oCamera.setPerspective(glm::radians(50.0f), 1280.0f / 720.0f, 0.1f, 20000.0f);

	m_file = new FBXFile;
	m_file->load("./data/models/characters/Pyro/pyro.fbx");
	m_file->initialiseOpenGLTextures();
	GenerateGLMeshes(m_file);
	LoadShader("./data/shaders/skinned_vertex.glsl", "", "./data/shaders/skinned_fragment.glsl", &m_programID);

	TwBar* m_bar = TwNewBar("Settings");
	TwAddVarRW(m_bar, "Ambient", TW_TYPE_COLOR3F, &m_vAmbCol, "group=Light");
	TwAddVarRW(m_bar, "Direction", TW_TYPE_DIR3F, &m_vLightPos, "group=Light");
	TwAddVarRW(m_bar, "Color", TW_TYPE_COLOR3F, &m_vLightCol, "group=Light");
	TwAddVarRW(m_bar, "Specular Power", TW_TYPE_FLOAT, &m_fSpecPow, "group=Light min=0.05 step=0.05 max=1000.0");

	Gizmos::create();
	return true;
}
bool Animation::shutdown(){
	TwTerminate();
	return Application::shutdown();
}
bool Animation::update(){
	if (!Application::update()){
		return false;
	}
	m_oCamera.update(m_fDeltaTime);

	if (glfwGetKey(m_window, GLFW_KEY_R) == GLFW_PRESS){
		ReloadShader();
	}

	return true;
}
void Animation::draw(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	Gizmos::clear();
	Gizmos::addTransform(mat4(1), 10);

	glUseProgram(m_programID);

	int view_proj_uniform = glGetUniformLocation(m_programID, "projView");
	int world_uniform = glGetUniformLocation(m_programID, "world");
	int bones_uniform = glGetUniformLocation(m_programID, "bones");

	int amb_color_uniform = glGetUniformLocation(m_programID, "ambCol");
	int dif_color_uniform = glGetUniformLocation(m_programID, "lightCol");
	int light_dir_uniform = glGetUniformLocation(m_programID, "lightDir");
	int cam_pos_uniform = glGetUniformLocation(m_programID, "camPos");
	int spec_pow_uniform = glGetUniformLocation(m_programID, "specPow");

	int diff_texture_uniform = glGetUniformLocation(m_programID, "diffTex");
	int norm_texture_uniform = glGetUniformLocation(m_programID, "normTex");
	int spec_texture_uniform = glGetUniformLocation(m_programID, "specTex");

	FBXSkeleton* skeleton = m_file->getSkeletonByIndex(0);
	FBXAnimation* anim = m_file->getAnimationByIndex(0);
	UpdateBones(skeleton);
	EvaluateSkeleton(anim, skeleton, m_fCurrTime);

	if (view_proj_uniform > -1)
		glUniformMatrix4fv(view_proj_uniform, 1, GL_FALSE, (float*)&m_oCamera.getProjectionView());

	if (amb_color_uniform > -1)
		glUniform3fv(amb_color_uniform, 1, (float*)&m_vAmbCol);
	if (dif_color_uniform > -1)
		glUniform3fv(dif_color_uniform, 1, (float*)&m_vLightCol);
	if (light_dir_uniform > -1)
		glUniform3fv(light_dir_uniform, 1, (float*)&(glm::normalize(-m_vLightPos)));
	if (cam_pos_uniform > -1)
		glUniform3fv(cam_pos_uniform, 1, (float*)&m_oCamera.getWorldTransform()[3].xyz);
	if (spec_pow_uniform > -1)
		glUniform1f(spec_pow_uniform, m_fSpecPow);

	if (diff_texture_uniform > -1)
		glUniform1i(diff_texture_uniform, 0);
	if (norm_texture_uniform > -1)
		glUniform1i(norm_texture_uniform, 1);
	if (spec_texture_uniform > -1)
		glUniform1i(spec_texture_uniform, 2);

	for (unsigned int i = 0; i < m_meshes.size(); ++i){

		FBXMeshNode* currMesh = m_file->getMeshByIndex(i);

		if (world_uniform > -1) {
			glUniformMatrix4fv(world_uniform, 1, GL_FALSE, (float*)&currMesh->m_globalTransform);
		}

		FBXMaterial* meshMat = currMesh->m_material;
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, meshMat->textures[FBXMaterial::DiffuseTexture]->handle);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, meshMat->textures[FBXMaterial::NormalTexture]->handle);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, meshMat->textures[FBXMaterial::SpecularTexture]->handle);



		glBindVertexArray(m_meshes[i].m_VAO);
		glDrawElements(GL_TRIANGLES, m_meshes[i].m_indexCount, GL_UNSIGNED_INT, 0);
	}

	if (bones_uniform > -1){
		glUniformMatrix4fv(bones_uniform, skeleton->m_boneCount, GL_FALSE, (float*)skeleton->m_bones);
	}

	for (unsigned int i = 0; i < skeleton->m_boneCount; ++i){
		skeleton->m_nodes[i]->updateGlobalTransform();
		mat4 node_global = skeleton->m_nodes[i]->m_globalTransform;
		vec3 node_pos = node_global[3].xyz;

		Gizmos::addAABBFilled(node_pos, vec3(4.f), vec4(1, 0, 0, 1), &node_global);

		if (skeleton->m_nodes[i]->m_parent != nullptr){
			vec3 parent_pos = skeleton->m_nodes[i]->m_parent->m_globalTransform[3].xyz;
			Gizmos::addLine(node_pos, parent_pos, vec4(0, 1, 0, 1));
		}
	}

	vec4 white(1);
	vec4 black(0, 0, 0, 1);

	for (int i = 0; i <= 20; ++i){
		Gizmos::addLine(vec3(-10 + i, 0, -10), i != 10 ? vec3(-10 + i, 0, 10) : vec3(-10 + i, 0, 0), i != 10 ? black : white);
		Gizmos::addLine(vec3(-10, 0, -10 + i), i != 10 ? vec3(10, 0, -10 + i) : vec3(0, 0, -10 + i), i != 10 ? black : white);
	}

	Gizmos::draw(m_oCamera.getProjectionView());
	TwDraw();
	Application::draw();
}

void Animation::GenerateGLMeshes(FBXFile* fbx){
	unsigned int meshCount = fbx->getMeshCount();

	m_meshes.resize(meshCount);

	for (unsigned int i = 0; i < meshCount; ++i){
		FBXMeshNode* currMesh = fbx->getMeshByIndex(i);

		m_meshes[i].m_indexCount = currMesh->m_indices.size();

		glGenBuffers(1, &m_meshes[i].m_VBO);
		glGenBuffers(1, &m_meshes[i].m_IBO);
		glGenVertexArrays(1, &m_meshes[i].m_VAO);

		glBindVertexArray(m_meshes[i].m_VAO);
		glBindBuffer(GL_ARRAY_BUFFER, m_meshes[i].m_VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(FBXVertex) * currMesh->m_vertices.size(), currMesh->m_vertices.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_meshes[i].m_IBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)* currMesh->m_indices.size(), currMesh->m_indices.data(), GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);//pos
		glEnableVertexAttribArray(1);//texcoord
		glEnableVertexAttribArray(2);//bone indices
		glEnableVertexAttribArray(3);//bone weights
		glEnableVertexAttribArray(4);//normal
		glEnableVertexAttribArray(5);//tangent

		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(FBXVertex), (void*)FBXVertex::PositionOffset);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(FBXVertex), (void*)FBXVertex::TexCoord1Offset);
		glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(FBXVertex), (void*)FBXVertex::IndicesOffset);
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(FBXVertex), (void*)FBXVertex::WeightsOffset);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_TRUE, sizeof(FBXVertex), (void*)(FBXVertex::NormalOffset));
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_TRUE, sizeof(FBXVertex), (void*)(FBXVertex::TangentOffset));

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
}

void Animation::EvaluateSkeleton(FBXAnimation* anim, FBXSkeleton* skeleton, float time){
	float fps = 24.f;
	int currTime = (int)(time * fps);
	//Loop thru tracks
	for (unsigned int i = 0; i < anim->m_trackCount; ++i) {
		//Get the correct bone for the given track.
		int trackFrameCount = anim->m_tracks[i].m_keyframeCount;
		int trackTime = currTime % trackFrameCount;

		//Find keyframes affecting bone.
		FBXKeyFrame currFrame = anim->m_tracks[i].m_keyframes[trackTime];
		FBXKeyFrame nextFrame = anim->m_tracks[i].m_keyframes[(trackTime + 1) % trackFrameCount];

		//interpolate between these keyframes to generate matrix for current pose
		float timeSinceFrame = time - (currTime / fps);
		float t = timeSinceFrame * fps;

		vec3 newPos = glm::mix(currFrame.m_translation, nextFrame.m_translation, t);
		vec3 newScale = glm::mix(currFrame.m_scale, nextFrame.m_scale, t);
		quat newRot = glm::slerp(currFrame.m_rotation, nextFrame.m_rotation, t);

		//set the fbxnodes local transforms to match
		mat4 transform = glm::translate(newPos) * glm::toMat4(newRot) * glm::scale(newScale);
		unsigned int boneIndex = anim->m_tracks[i].m_boneIndex;

		if (boneIndex < skeleton->m_boneCount)
			skeleton->m_nodes[boneIndex]->m_localTransform = transform;
	}
}

void Animation::UpdateBones(FBXSkeleton* skeleton){
	//loop thru nodes in skele
	for (unsigned int i = 0; i < skeleton->m_boneCount; ++i) {
		//generate their global transforms 
		int parentIndex = skeleton->m_parentIndex[i];
		if (parentIndex == -1) {
			skeleton->m_bones[i] = skeleton->m_nodes[i]->m_localTransform;
		}
		else {
			skeleton->m_bones[i] = skeleton->m_bones[parentIndex] * skeleton->m_nodes[i]->m_localTransform;
		}
	}
	for (unsigned int i = 0; i < skeleton->m_boneCount; ++i) {
		skeleton->m_bones[i] *= skeleton->m_bindPoses[i];
	}
}

void Animation::ReloadShader(){
	glDeleteProgram(m_programID);
	LoadShader("./data/shaders/skinned_vertex.glsl", "", "./data/shaders/skinned_fragment.glsl", &m_programID);
}