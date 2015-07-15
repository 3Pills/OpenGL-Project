#ifndef _FBX_MODEL_H_
#define _FBX_MODEL_H_
#include <FBXFile.h>
#include "Vertex.h"
#include <vector>
#include "Camera.h"

class FBXModel {
private:
	std::vector<OpenGLData> m_meshes;

	unsigned int m_instantRender, m_deferredRender;

	void EvaluateSkeleton(float dt);
	void UpdateBones();
public:
	FBXModel(const char* a_szModelPath);
	~FBXModel();

	FBXFile* m_file;
	FBXSkeleton* m_skeleton;
	FBXAnimation* m_anim;

	void GenerateGLMeshes(FBXFile* fbx);

	void Update(float dt);
	void Render(FlyCamera a_camera);
	void RenderDeferred(FlyCamera a_camera);

	void ReloadShader();
};

#endif//_FBX_MODEL_H_