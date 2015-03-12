#ifndef SCENE_MANAGEMENT_H_
#define SCENE_MANAGEMENT_H_
#include "Application.h"
#include "Camera.h"
#include "BoundingVolumes.h"

class SceneManagement : public Application
{
	FlyCamera m_oCamera;
public:
	SceneManagement();
	virtual ~SceneManagement();

	virtual bool startup();
	virtual bool shutdown();

	virtual bool update();
	virtual void draw();

	MeshObject LoadMesh(const char* obj_filename);
	void DrawMesh();
};

#endif//SCENE_MANAGEMENT_H_