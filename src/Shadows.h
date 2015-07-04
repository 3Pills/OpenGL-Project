#ifndef CAM_N_PROJ_H_
#define CAM_N_PROJ_H_
#include "Application.h"
#include "Camera.h"
#include "Vertex.h"

class Shadows : public Application
{
	FlyCamera m_oCamera;

	OpenGLData m_bunny;
	OpenGLData m_plane;

	vec3 m_lightDir;
	mat4 m_lightMatrix;

	unsigned int m_fbo, m_fboDepth;
	unsigned int m_shadowProgramID, m_diffuseShadowProgramID;
public:
	Shadows();
	virtual ~Shadows();

	virtual bool startup();
	virtual bool shutdown();
	virtual bool update();
	virtual void draw();

	void BuildMeshes();
	void BuildShadowMap();
	void ReloadShader();
};

#endif//CAM_N_PROJ_H_