#ifndef DEFERRED_RENDERING_H_
#define DEFERRED_RENDERING_H_
#include "Application.h"
#include "Camera.h"
#include "Vertex.h"

class DeferredRendering : public Application
{
	FlyCamera m_oCamera;

	unsigned int m_gBufferFBO, m_albedoTexture, m_positionTexture, m_normalTexture, m_depthTexture;
	unsigned int m_lightFBO, m_lightTexture;
	unsigned int m_gBufferProgram, m_compositeProgram, m_directionalLightProgram, m_pointLightProgram;

	OpenGLData m_bunny;
	OpenGLData m_lightCube;
	OpenGLData m_screenspaceQuad;
public:
	DeferredRendering();
	virtual ~DeferredRendering();

	virtual bool startup();
	virtual bool shutdown();

	virtual bool update();
	virtual void draw();

	void BuildMesh();
	void BuildQuad();
	void BuildCube();
	void BuildGBuffer();
	void BuildLightBuffer();

	void RenderDirectionalLight(vec3 a_lightDir, vec3 a_lightColor);
	void RenderPointLight(vec3 a_lightPos, float a_radius, vec3 a_lightColor);

	void ReloadShader();
};

#endif//DEFERRED_RENDERING_H_