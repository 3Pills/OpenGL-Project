#ifndef ANIMATION_H_
#define ANIMATION_H_
#include "Application.h"
#include "Camera.h"

#include "FBXFile.h"
#include "Vertex.h"
#include "AntTweakBar.h"

class Animation : public Application
{
	FlyCamera m_oCamera;

	FBXFile* m_file;
	
	std::vector<OpenGLData> m_meshes;
	unsigned int m_programID;
	vec3 m_vAmbCol, m_vLightCol, m_vLightPos;
	float m_fSpecPow;
public:
	Animation();
	virtual ~Animation();

	virtual bool startup();
	virtual bool shutdown();

	virtual bool update();
	virtual void draw();

	void GenerateGLMeshes(FBXFile* fbx);
	void EvaluateSkeleton(FBXAnimation* anim, FBXSkeleton* skeleton, float time);
	void UpdateBones(FBXSkeleton* skeleton);
	void ReloadShader();
};

#endif//ANIMATION_H_