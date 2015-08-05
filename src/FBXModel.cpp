//TODO: Fill this out.
#include "FBXModel.h"
#include "Utility.h"

FBXModel::FBXModel(const char* a_szModelPath, float a_roughness, float a_fresnelScale, mat4 a_modTransform, vec3 a_pos, vec3 a_scale, quat a_rot)
: m_roughness(a_roughness), m_fresnelScale(a_fresnelScale), m_pos(a_pos), m_modTransform(a_modTransform), m_scale(a_scale), m_rot(a_rot), m_animationTime(0),
m_animationTimeStep(1), m_parentTransform(true){
	m_file = new FBXFile;
	m_file->load(a_szModelPath);
	m_file->initialiseOpenGLTextures();

	GenerateGLMeshes(m_file);
	SetAnimation(eRUN);

	LoadShader("./data/shaders/skinned_vertex.glsl", 0, "./data/shaders/skinned_fragment_pbr.glsl", &m_shaders[0]);
	LoadShader("./data/shaders/skinned_vertex.glsl", 0, "./data/shaders/gbuffer_textured_fragment.glsl", &m_shaders[1]);
	LoadShader("./data/shaders/skinned_vertex.glsl", 0, "./data/shaders/shadowed_fragment.glsl", &m_shaders[2]);
	LoadShader("./data/shaders/skinned_vertex_anim.glsl", 0, "./data/shaders/skinned_fragment_pbr.glsl", &m_shaders[3]);
	LoadShader("./data/shaders/skinned_vertex_anim.glsl", 0, "./data/shaders/gbuffer_textured_fragment.glsl", &m_shaders[4]);
	LoadShader("./data/shaders/skinned_vertex_anim.glsl", 0, "./data/shaders/shadowed_fragment.glsl", &m_shaders[5]);
}

FBXModel::~FBXModel(){
	m_file->unload();
	for (int i = 0; i < 6; i++) 
		glDeleteProgram(m_shaders[i]);
}

//Generates OpenGL buffer for meshes in model.
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

//Computes final transform and animates any bone-rigged models.
void FBXModel::Update(float dt) {
	m_transform = glm::translate(m_pos) * glm::toMat4(m_rot) * glm::scale(m_scale) * m_modTransform;
	EvaluateSkeleton(dt);
	UpdateBones();
}

//Renders with individual shader data.
void FBXModel::Render(Camera* a_camera, bool a_deferred, mat4* a_lightMatrix) {
	int loc = -1;
	//Get the correct index in the shader program array, then use program
	unsigned int program = (m_file->getSkeletonCount() > 0) ? 3 : 0;
	if (a_deferred) { program += 1; }
	else if (a_lightMatrix != nullptr) { program += 2; }
	program = m_shaders[program];

	glUseProgram(program);

	//If a light matrix has been passed in, replace the camera projection view with the light's projection view.
	loc = glGetUniformLocation(program, "projView");
	glUniformMatrix4fv(loc, 1, GL_FALSE, (float*)&((a_lightMatrix != nullptr) ? *a_lightMatrix : a_camera->GetProjectionView()));

	//If the framebuffer is not being deferred pass an identity matrix as the view matrix.
	loc = glGetUniformLocation(program, "view");
	glUniformMatrix4fv(loc, 1, GL_FALSE, (float*)&((a_deferred) ? a_camera->GetView() : mat4(1)));

	loc = glGetUniformLocation(program, "ambCol");
	glUniform3fv(loc, 1, (float*)&m_ambCol);
	
	loc = glGetUniformLocation(program, "lightCol");
	glUniform3fv(loc, 1, (float*)&m_lightCol);
	
	loc = glGetUniformLocation(program, "lightDir");
	glUniform3fv(loc, 1, (float*)&(glm::normalize(m_lightDir)));

	loc = glGetUniformLocation(program, "camPos");
	glUniform3fv(loc, 1, (float*)&a_camera->GetWorldTransform()[3].xyz);

	loc = glGetUniformLocation(program, "transform");
	glUniformMatrix4fv(loc, 1, GL_FALSE, (float*)&m_transform);

	if (m_file->getSkeletonCount() > 0) {
		loc = glGetUniformLocation(program, "bones");
		glUniformMatrix4fv(loc, m_skeleton->m_boneCount, GL_FALSE, (float*)m_skeleton->m_bones);
	}

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

	for (unsigned int i = 0; i < m_meshes.size(); ++i){
		FBXMeshNode* currMesh = m_file->getMeshByIndex(i);
		FBXMaterial* meshMat = currMesh->m_material;
		if (meshMat->textures[FBXMaterial::DiffuseTexture] != nullptr) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, meshMat->textures[FBXMaterial::DiffuseTexture]->handle);
		}
		if (meshMat->textures[FBXMaterial::NormalTexture] != nullptr) {
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, meshMat->textures[FBXMaterial::NormalTexture]->handle);
		}
		if (meshMat->textures[FBXMaterial::SpecularTexture] != nullptr) {
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, meshMat->textures[FBXMaterial::SpecularTexture]->handle);
		}

		glBindVertexArray(m_meshes[i].m_VAO);
		glDrawElements(GL_TRIANGLES, m_meshes[i].m_indexCount, GL_UNSIGNED_INT, 0);
	}
}

//Moves the bones in a skeleton to a position based on current time
void FBXModel::EvaluateSkeleton(float dt){
	//Ignore models without animations.
	if (m_file->getSkeletonCount() == 0 || m_file->getAnimationCount() == 0)
		return;

	m_skeleton = m_file->getSkeletonByIndex(0);
	m_anim = m_file->getAnimationByIndex(0);

	m_animationTime += (dt * m_animationTimeStep > 0.3f) ? 0.3f : dt * m_animationTimeStep;

	if (m_animationTime < m_startTime) m_animationTime = m_startTime;
	m_skeleton->evaluate(m_anim, m_animationTime);
	if (m_animationTime > m_endTime) m_animationTime = m_startTime;
}

//Sets the start time and end time of an animated model
void FBXModel::SetAnimation(const int animationID) {
	//Currently uses constant values for the pyro model. No other way to get each individual animation.
	if (animationID == eIDLE) {
		m_startTime = 0.1f;
		m_endTime = 3.85f;
		m_animationTimeStep = 1.0f;
	}
	if (animationID == eRUN) {
		m_startTime = 41.4f;
		m_endTime = 42.388f;
		m_animationTimeStep = 1.25f;
	}
}

//Updates the transforms of each bone
void FBXModel::UpdateBones(){
	//Ignore models without bones.
	if (m_file->getSkeletonCount() == 0)
		return;

	m_skeleton = m_file->getSkeletonByIndex(0);
	m_skeleton->updateBones();
}

//Reload the shaders in the model.
void FBXModel::ReloadShader(){
	for (int i = 0; i < 6; i++)
		glDeleteProgram(m_shaders[i]);

	LoadShader("./data/shaders/skinned_vertex.glsl", 0, "./data/shaders/skinned_fragment_pbr.glsl", &m_shaders[0]);
	LoadShader("./data/shaders/skinned_vertex.glsl", 0, "./data/shaders/gbuffer_textured_fragment.glsl", &m_shaders[1]);
	LoadShader("./data/shaders/skinned_vertex.glsl", 0, "./data/shaders/shadowed_fragment.glsl", &m_shaders[2]);
	LoadShader("./data/shaders/skinned_vertex_anim.glsl", 0, "./data/shaders/skinned_fragment_pbr.glsl", &m_shaders[3]);
	LoadShader("./data/shaders/skinned_vertex_anim.glsl", 0, "./data/shaders/gbuffer_textured_fragment.glsl", &m_shaders[4]);
	LoadShader("./data/shaders/skinned_vertex_anim.glsl", 0, "./data/shaders/shadowed_fragment.glsl", &m_shaders[5]);
}