#ifndef SCENE_MANAGEMENT_H_
#define SCENE_MANAGEMENT_H_
#include "Application.h"
#include "Camera.h"
#include "BoundingVolumes.h"

class SceneManagement : public Application
{
	FlyCamera m_oCamera;
	std::vector<MeshObject> m_meshes;

	vec3 m_ambCol, m_lightCol, m_matCol, m_lightDir;
	float m_specPow;

	unsigned int m_programID;
public:
	SceneManagement();
	virtual ~SceneManagement();

	virtual bool startup();
	virtual bool shutdown();

	virtual bool update();
	virtual void draw();

	void LoadMesh(const char* obj_filename);
	void DrawMesh(MeshObject a_mesh);
};

#endif//SCENE_MANAGEMENT_H_