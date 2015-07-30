#ifndef _FBX_MODEL_H_
#define _FBX_MODEL_H_
#include <FBXFile.h>
#include "Vertex.h"
#include <vector>
#include "Camera.h"

class FBXModel {
protected:
	std::vector<OpenGLData> m_meshes;

	FBXSkeleton* m_skeleton;
	FBXAnimation* m_anim;

	unsigned int m_pbrShader, m_phongShader, m_deferredShader;
	float m_animationTime;

	bool m_pbr;

	void EvaluateSkeleton(float dt);
	void UpdateBones();
public:
	FBXModel(const char* a_szModelPath, float a_roughness = 0.3f, float a_fresnelScale = 2.0f, 
			 mat4 a_modTransform = mat4(1), vec3 a_pos = vec3(0), vec3 a_scale = vec3(1), quat a_rot = quat::tquat());
	virtual ~FBXModel();

	quat m_rot;
	//m_transform is the final transform of the model. modTransform is a constant which modifies objects (to keep them to a world scale)
	mat4 m_transform, m_modTransform;
	vec3 m_lightDir, m_lightCol, m_ambCol, m_pos, m_scale;
	float m_specPow, m_roughness, m_fresnelScale;

	FBXFile* m_file;

	void GenerateGLMeshes(FBXFile* fbx);

	virtual void Update(float dt);

	void Render(FlyCamera a_camera);
	void RenderDeferred(FlyCamera a_camera);

	virtual void RenderGizmos();

	void ReloadShader();
};

#endif//_FBX_MODEL_H_