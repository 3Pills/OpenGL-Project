#ifndef _FBX_MODEL_H_
#define _FBX_MODEL_H_
#include <FBXFile.h>
#include "Vertex.h"
#include <vector>
#include "Camera.h"

class FBXModel {
private:
	std::vector<OpenGLData> m_meshes;

	FBXFile* m_file;
	FBXSkeleton* m_skeleton;
	FBXAnimation* m_anim;

	unsigned int m_pbrShader, m_phongShader, m_deferredShader;

	bool m_pbr;

	void EvaluateSkeleton(float dt);
	void UpdateBones();
public:
	FBXModel(const char* a_szModelPath);
	~FBXModel();

	vec3 m_lightDir, m_lightCol, m_ambCol;
	float m_specPow, m_roughness, m_fresnelScale;

	void GenerateGLMeshes(FBXFile* fbx);

	void Update(float dt);
	void Render(FlyCamera a_camera);
	void RenderDeferred(FlyCamera a_camera);

	void ReloadShader();
};

#endif//_FBX_MODEL_H_