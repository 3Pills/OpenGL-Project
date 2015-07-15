#ifndef VIRTUAL_WORLD_H_
#define VIRTUAL_WORLD_H_
#include "Application.h"
#include "Camera.h"
#include <vector>
#include "GPUEmitter.h"
#include "FBXModel.h"

class VirtualWorld : public Application
{
	FlyCamera m_oCamera;
	std::vector<GPUEmitter*> m_aParticleEmitters;
	std::vector<FBXModel*> m_aFBXModels;

	OpenGLData m_screenspaceQuad;
	OpenGLData m_lightCube;

	unsigned int m_gBufferFBO, m_albedoTexture, m_positionTexture, m_normalTexture, m_depthTexture, m_lightFBO, m_lightTexture;
	unsigned int m_gBufferProgram, m_compositeProgram, m_dirLightProgram, m_pointLightProgram;
	unsigned int m_LastKey;
public:
	VirtualWorld();
	virtual ~VirtualWorld();

	virtual bool startup();
	virtual bool shutdown();

	virtual bool update();
	virtual void draw();

	void BuildGBuffer();
	void BuildLightBuffer();
	void BuildQuad();
	void BuildCube();

	void RenderDirectionalLight(vec3 a_lightDir, vec3 a_lightColor);
	void RenderPointLight(vec3 a_lightPos, float a_radius, vec3 a_lightColor);

	void ReloadShaders();
};

#endif//VIRTUAL_WORLD_H_