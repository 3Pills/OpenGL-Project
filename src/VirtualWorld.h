#ifndef VIRTUAL_WORLD_H_
#define VIRTUAL_WORLD_H_
#include "Application.h"
#include "Camera.h"
#include <vector>
#include "GPUEmitter.h"
#include "FBXModel.h"

//Struct for containing point light data.
struct PointLight {
	vec3 m_position;
	vec3 m_color;
	float m_radius;

	PointLight(vec3 a_position, vec3 a_color, float a_radius) : m_position(a_position), m_color(a_color), m_radius(a_radius) {}
};

class VirtualWorld : public Application
{
	FlyCamera m_oCamera;
	std::vector<GPUEmitter*> m_particleEmitters;
	std::vector<FBXModel*> m_FBXModels;
	std::vector<PointLight*> m_pointLights;

	OpenGLData m_screenspaceQuad;
	OpenGLData m_lightCube;
	OpenGLData m_planeMesh;

	float* m_perlinData;

	unsigned int m_gBufferFBO, m_albedoTexture, m_positionTexture, m_normalTexture, m_depthTexture, m_lightFBO, m_lightTexture, m_perlinTexture;
	unsigned int m_gBufferProgram, m_compositeProgram, m_dirLightProgram, m_pointLightProgram, m_proceduralProgram;
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
	void BuildProceduralGrid(vec2 a_realDims, glm::ivec2 a_dims);
	void BuildPerlinTexture(glm::ivec2 a_dims, const int a_octaves, const float a_amplitude, const float a_persistance);

	void RenderDirectionalLight(vec3 a_lightDir, vec3 a_lightColor);
	void RenderPointLight(vec3 a_lightPos, float a_radius, vec3 a_lightColor);

	void ReloadShaders();
};

#endif//VIRTUAL_WORLD_H_