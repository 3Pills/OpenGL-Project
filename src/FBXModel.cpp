//TODO: Fill this out.
#include "FBXModel.h"
#include "Utility.h"

FBXModel::FBXModel(const char* a_szModelPath, vec3 a_pos, float a_roughness, float a_fresnelScale, vec3 a_scale, quat a_rot) 
	: m_pbr(true), m_fresnelScale(a_roughness), m_roughness(a_fresnelScale), m_pos(a_pos), m_scale(a_scale), m_rot(a_rot), m_animationTime(0){
	m_file = new FBXFile;
	m_file->load(a_szModelPath);
	m_file->initialiseOpenGLTextures();

	GenerateGLMeshes(m_file);

	LoadShader("./data/shaders/skinned_vertex.glsl", 0, "./data/shaders/skinned_fragment_pbr.glsl", &m_pbrShader);
	LoadShader("./data/shaders/skinned_vertex.glsl", 0, "./data/shaders/skinned_fragment_phong.glsl", &m_phongShader);
	LoadShader("./data/shaders/skinned_vertex.glsl", 0, "./data/shaders/gbuffer_textured_fragment.glsl", &m_deferredShader);
}

FBXModel::~FBXModel(){
	m_file->unload();
	glDeleteProgram(m_pbrShader);
	glDeleteProgram(m_phongShader);
	glDeleteProgram(m_deferredShader);
}

void FBXModel::GenerateGLMeshes(FBXFile* fbx){
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

void FBXModel::Update(float dt) {
	m_transform = glm::translate(m_pos) * glm::toMat4(m_rot) * glm::scale(m_scale);
	EvaluateSkeleton(dt);
	UpdateBones();
}

//Renders with individual shader data.
void FBXModel::Render(FlyCamera a_camera) {
	unsigned int program = (m_pbr) ? m_pbrShader : m_phongShader;

	glUseProgram(program);

	int loc = glGetUniformLocation(program, "projView");
	glUniformMatrix4fv(loc, 1, GL_FALSE, (float*)&a_camera.getProjectionView());

	loc = glGetUniformLocation(program, "ambCol");
	glUniform3fv(loc, 1, (float*)&m_ambCol);
	
	loc = glGetUniformLocation(program, "lightCol");
	glUniform3fv(loc, 1, (float*)&m_lightCol);
	
	loc = glGetUniformLocation(program, "lightDir");
	glUniform3fv(loc, 1, (float*)&(glm::normalize(m_lightDir)));

	loc = glGetUniformLocation(program, "camPos");
	glUniform3fv(loc, 1, (float*)&a_camera.getWorldTransform()[3].xyz);

	loc = glGetUniformLocation(program, "transform");
	glUniformMatrix4fv(loc, 1, GL_FALSE, (float*)&m_transform);

	loc = glGetUniformLocation(program, "specPow");
	glUniform1f(loc, m_specPow);

	loc = glGetUniformLocation(program, "roughness");
	glUniform1f(loc, m_roughness);

	loc = glGetUniformLocation(program, "fresnelScale");
	glUniform1f(loc, m_fresnelScale);

	loc = glGetUniformLocation(program, "diffuse");
	glUniform1i(loc, 0);

	loc = glGetUniformLocation(program, "normal");
	glUniform1i(loc, 1);

	loc = glGetUniformLocation(program, "specular");
	glUniform1i(loc, 2);

	if (m_file->getSkeletonCount() > 0) {
		loc = glGetUniformLocation(program, "bones");
		glUniformMatrix4fv(loc, m_skeleton->m_boneCount, GL_FALSE, (float*)m_skeleton->m_bones);
	}

	loc = glGetUniformLocation(program, "hasBones");
	glUniform1i(loc, (m_file->getSkeletonCount() > 0));

	for (unsigned int i = 0; i < m_meshes.size(); ++i){
		FBXMeshNode* currMesh = m_file->getMeshByIndex(i);

		loc = glGetUniformLocation(program, "world");
		glUniformMatrix4fv(loc, 1, GL_FALSE, (float*)&currMesh->m_globalTransform);

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
}

//Renders with deferred shader data.
void FBXModel::RenderDeferred(FlyCamera a_camera) {
	glUseProgram(m_deferredShader);

	int loc = glGetUniformLocation(m_deferredShader, "view");
	glUniformMatrix4fv(loc, 1, GL_FALSE, (float*)&a_camera.getView());

	loc = glGetUniformLocation(m_deferredShader, "projView");
	glUniformMatrix4fv(loc, 1, GL_FALSE, (float*)&a_camera.getProjectionView());

	if (m_file->getSkeletonCount() > 0) {
		loc = glGetUniformLocation(m_deferredShader, "bones");
		glUniformMatrix4fv(loc, m_skeleton->m_boneCount, GL_FALSE, (float*)m_skeleton->m_bones);
	}

	loc = glGetUniformLocation(m_deferredShader, "hasBones");
	glUniform1i(loc, (m_file->getSkeletonCount() > 0));

	loc = glGetUniformLocation(m_deferredShader, "camPos");
	glUniform3fv(loc, 1, (float*)&a_camera.getWorldTransform()[3].xyz);

	loc = glGetUniformLocation(m_deferredShader, "transform");
	glUniformMatrix4fv(loc, 1, GL_FALSE, (float*)&m_transform);

	loc = glGetUniformLocation(m_deferredShader, "roughness");
	glUniform1f(loc, m_roughness);

	loc = glGetUniformLocation(m_deferredShader, "fresnelScale");
	glUniform1f(loc, m_fresnelScale);

	loc = glGetUniformLocation(m_deferredShader, "deferred");
	glUniform1i(loc, true);

	loc = glGetUniformLocation(m_deferredShader, "diffuse");
	glUniform1i(loc, 0);

	loc = glGetUniformLocation(m_deferredShader, "normal");
	glUniform1i(loc, 1);

	loc = glGetUniformLocation(m_deferredShader, "specular");
	glUniform1i(loc, 2);

	for (unsigned int i = 0; i < m_meshes.size(); ++i){
		FBXMeshNode* currMesh = m_file->getMeshByIndex(i);

		loc = glGetUniformLocation(m_deferredShader, "world");
		glUniformMatrix4fv(loc, 1, GL_FALSE, (float*)&currMesh->m_globalTransform);

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
}

void FBXModel::RenderGizmos(){}

void FBXModel::EvaluateSkeleton(float dt){
	//Ignore models without animations.
	if (m_file->getSkeletonCount() == 0 || m_file->getAnimationCount() == 0)
		return;

	m_skeleton = m_file->getSkeletonByIndex(0);
	m_anim = m_file->getAnimationByIndex(0);

	m_animationTime += (dt > 0.3) ? 0.3 : dt;

	float fps = 24.0f;
	int currTime = (int)(m_animationTime * fps);
	//Loop thru tracks
	for (unsigned int i = 0; i < m_anim->m_trackCount; ++i) {
		//Get the correct bone for the given track.
		int trackFrameCount = m_anim->m_tracks[i].m_keyframeCount;

		int trackTime = currTime % trackFrameCount;
		//if (trackTime == 42) {
		//	m_animationTime = 0;
		//}

		//Find keyframes affecting bone.
		FBXKeyFrame currFrame = m_anim->m_tracks[i].m_keyframes[trackTime];
		FBXKeyFrame nextFrame = m_anim->m_tracks[i].m_keyframes[(trackTime + 1) % trackFrameCount];


		//interpolate between these keyframes to generate matrix for current pose
		float timeSinceFrame = m_animationTime - (currTime / fps);
		float t = timeSinceFrame * fps;

		vec3 newPos = glm::mix(currFrame.m_translation, nextFrame.m_translation, t);
		vec3 newScale = glm::mix(currFrame.m_scale, nextFrame.m_scale, t);
		quat newRot = glm::slerp(currFrame.m_rotation, nextFrame.m_rotation, t);

		//set the fbxnodes local transforms to match
		mat4 transform = glm::translate(newPos) * glm::toMat4(newRot) * glm::scale(newScale);
		unsigned int boneIndex = m_anim->m_tracks[i].m_boneIndex;

		if (boneIndex < m_skeleton->m_boneCount)
			m_skeleton->m_nodes[boneIndex]->m_localTransform = transform;
	}
}

void FBXModel::UpdateBones(){
	//Ignore models without bones.
	if (m_file->getSkeletonCount() == 0)
		return;

	m_skeleton = m_file->getSkeletonByIndex(0);
	//loop thru nodes in skele
	for (unsigned int i = 0; i < m_skeleton->m_boneCount; ++i) {
		//generate their global transforms 
		int parentIndex = m_skeleton->m_parentIndex[i];
		if (parentIndex == -1) {
			m_skeleton->m_bones[i] = m_skeleton->m_nodes[i]->m_localTransform;
		}
		else {
			m_skeleton->m_bones[i] = m_skeleton->m_bones[parentIndex] * m_skeleton->m_nodes[i]->m_localTransform;
		}
	}
	for (unsigned int i = 0; i < m_skeleton->m_boneCount; ++i) {
		m_skeleton->m_bones[i] *= m_skeleton->m_bindPoses[i];
		m_skeleton->m_nodes[i]->updateGlobalTransform();
	}
}

void FBXModel::ReloadShader(){
	glDeleteProgram(m_pbrShader);
	glDeleteProgram(m_phongShader);
	glDeleteProgram(m_deferredShader);
	LoadShader("./data/shaders/skinned_vertex.glsl", 0, "./data/shaders/skinned_fragment_pbr.glsl", &m_pbrShader);
	LoadShader("./data/shaders/skinned_vertex.glsl", 0, "./data/shaders/skinned_fragment_phong.glsl", &m_phongShader);
	LoadShader("./data/shaders/skinned_vertex.glsl", "", "./data/shaders/gbuffer_textured_fragment.glsl", &m_deferredShader);
}