#ifndef _FBX_MODEL_H_
#define _FBX_MODEL_H_
#include <FBXFile.h>
#include "Vertex.h"
#include <vector>
#include "Camera.h"

enum FBXAnimID {
	eIDLE = 0,
	eRUN = 1
};

class FBXModel {
protected:
	std::vector<OpenGLData> m_meshes;

	FBXSkeleton* m_skeleton; //Pointer to skeleton object in FBXFile
	FBXAnimation* m_anim; //Pointer to animation object in FBXFile

	unsigned int m_shaders[6]; //Shader programs
	float m_animationTime; //Current time in animation
	float m_animationTimeStep; //Speed factor of animation
	float m_startTime, m_endTime; //Time clamp for animation

	mat4 m_transform; //Final transform of the model, calculated during update.

	void EvaluateSkeleton(float dt);
	void UpdateBones();
public:
	FBXModel(const char* a_szModelPath, float a_roughness = 0.3f, float a_fresnelScale = 2.0f, 
			 mat4 a_modTransform = mat4(1), vec3 a_pos = vec3(0), vec3 a_scale = vec3(1), quat a_rot = quat::tquat());
	virtual ~FBXModel();

	vec3 m_scale; //The scale transform of the model
	quat m_rot; //The rotation transform of the model
	vec3 m_pos; //The position (translation) transform of the model
	mat4 m_modTransform; //Modifies the final transform.

	vec3 m_lightDir; //Direction for light hitting the model. Only applied if not renderring deferred.
	vec3 m_lightCol; //Color for light hitting the model. Only applied if not renderring deferred.
	vec3 m_ambCol; //Ambient color of the model. Only applied if not renderring deferred.
	
	float m_specPow; //Specular power of lighting on the model
	float m_roughness; //Roughness value of lighting on the model.
	float m_fresnelScale; //Fresnel scale of lighting on the model.
	bool m_parentTransform; //For disabling parenting rotation to PhysX models (mainly for player).

	FBXFile* m_file; //Main access for FBX File data.

	void GenerateGLMeshes(FBXFile* fbx);

	virtual void Update(float dt);

	void Render(Camera* a_camera, bool a_deferred = false, mat4* a_projView = nullptr);
	void ReloadShader();

	void SetAnimation(const int a_animationID);
};

#endif//_FBX_MODEL_H_