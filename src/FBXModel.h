#ifndef _FBX_MODEL_H_
#define _FBX_MODEL_H_
#include <FBXFile.h>

class FBXModel {
private:

public:
	void GenerateGLMeshes(FBXFile* fbx);
	void EvaluateSkeleton(FBXAnimation* anim, FBXSkeleton* skeleton, float time);
	void UpdateBones(FBXSkeleton* skeleton);
};

#endif//_FBX_MODEL_H_