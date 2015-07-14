//TODO: Fill this out.
#include "FBXModel.h"
#include "Utility.h"

FBXModel::FBXModel(const char* a_szModelPath) {
	m_file = new FBXFile;
	m_file->load(a_szModelPath);
	m_file->initialiseOpenGLTextures();

	GenerateGLMeshes(m_file);

	LoadShader("./data/shaders/skinned_vertex.glsl", "", "./data/shaders/skinned_fragment.glsl", &m_programID);
}

FBXModel::~FBXModel(){
	m_file->unload();
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
	EvaluateSkeleton(dt);
	UpdateBones();
}

void FBXModel::Render(FlyCamera a_camera) {
	glUseProgram(m_programID);

	int loc = glGetUniformLocation(m_programID, "projView");
	glUniformMatrix4fv(loc, 1, GL_FALSE, (float*)&a_camera.getProjectionView());

	loc = glGetUniformLocation(m_programID, "ambCol");
	glUniform3fv(loc, 1, (float*)&vec3(1));
	
	loc = glGetUniformLocation(m_programID, "lightCol");
	glUniform3fv(loc, 1, (float*)&vec3(1));
	
	loc = glGetUniformLocation(m_programID, "lightDir");
	glUniform3fv(loc, 1, (float*)&(glm::normalize(-vec3(1))));

	loc = glGetUniformLocation(m_programID, "camPos");
	glUniform3fv(loc, 1, (float*)&a_camera.getWorldTransform()[3].xyz);

	loc = glGetUniformLocation(m_programID, "specPow");
	glUniform1f(loc, 1);

	loc = glGetUniformLocation(m_programID, "diffTex");
	glUniform1i(loc, 0);

	loc = glGetUniformLocation(m_programID, "normTex");
	glUniform1i(loc, 1);

	loc = glGetUniformLocation(m_programID, "specTex");
	glUniform1i(loc, 2);

	loc = glGetUniformLocation(m_programID, "bones");
	glUniformMatrix4fv(loc, m_skeleton->m_boneCount, GL_FALSE, (float*)m_skeleton->m_bones);

	for (unsigned int i = 0; i < m_meshes.size(); ++i){
		FBXMeshNode* currMesh = m_file->getMeshByIndex(i);

		loc = glGetUniformLocation(m_programID, "world");
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

void FBXModel::EvaluateSkeleton(float time){
	m_skeleton = m_file->getSkeletonByIndex(0);
	m_anim = m_file->getAnimationByIndex(0);

	float fps = 24.f;
	int currTime = (int)(time * fps);
	//Loop thru tracks
	for (unsigned int i = 0; i < m_anim->m_trackCount; ++i) {
		//Get the correct bone for the given track.
		int trackFrameCount = m_anim->m_tracks[i].m_keyframeCount;
		int trackTime = currTime % trackFrameCount;

		//Find keyframes affecting bone.
		FBXKeyFrame currFrame = m_anim->m_tracks[i].m_keyframes[trackTime];
		FBXKeyFrame nextFrame = m_anim->m_tracks[i].m_keyframes[(trackTime + 1) % trackFrameCount];

		//interpolate between these keyframes to generate matrix for current pose
		float timeSinceFrame = time - (currTime / fps);
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
	glDeleteProgram(m_programID);
	LoadShader("./data/shaders/skinned_vertex.glsl", "", "./data/shaders/skinned_fragment.glsl", &m_programID);
}