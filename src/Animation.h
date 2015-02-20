#ifndef ANIMATION_H_
#define ANIMATION_H_
#include "Application.h"
#include "Camera.h"

#include "FBXFile.h"
#include "Vertex.h"

class Animation : public Application
{
	FlyCamera m_oCamera;

	FBXFile* m_file;
	
	std::vector<OpenGLInfo> m_meshes;
	unsigned int m_programID;
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
};

#endif//ANIMATION_H_