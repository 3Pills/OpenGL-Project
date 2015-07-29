#ifndef VIRTUAL_WORLD_H_
#define VIRTUAL_WORLD_H_
#include "Application.h"
#include "Camera.h"
#include <vector>
#include "GPUEmitter.h"
#include "FBXModel.h"
#include "PhysModel.h"
#include "PhysScene.h"

//Struct for containing point light data.
struct PointLight {
	vec3 m_pos;
	vec3 m_color;
	float m_radius;

	PointLight(vec3 a_pos, vec3 a_color, float a_radius) : m_pos(a_pos), m_color(a_color), m_radius(a_radius) {}
};

struct DirectionalLight {
	vec3 m_dir;
	vec3 m_color;

	DirectionalLight(vec3 a_dir, vec3 a_color) : m_dir(a_dir), m_color(a_color) {}
};

class VirtualWorld : public Application
{
	FlyCamera m_oCamera;
	std::vector<GPUEmitter*> m_particleEmitters;
	std::vector<FBXModel*> m_FBXModels;
	std::vector<PhysModel*> m_PhysModels;

	OpenGLData m_screenspaceQuad;
	OpenGLData m_lightCube;
	OpenGLData m_planeMesh;

	PhysScene m_physScene;

	vec3 m_ambCol;

	vec2 m_perlinTextureSize, m_perlinWorldSize;

	bool m_debug[10];

	unsigned int m_gBufferFBO, m_albedoTexture, m_positionTexture, m_normalTexture, m_specularTexture, m_depthTexture; //G-Buffer data
	unsigned int m_lightFBO, m_lightTexture; //Light Buffer data
	unsigned int m_fxFBO, m_fxTexture; //Effects Buffer data

	unsigned int m_perlinTexture, m_pOct; //Procedural data
	float m_pScale, m_pAmp, m_pPers;
	float* m_perlinData;

	unsigned int m_gBufferProgram, m_compositeProgram, m_dirLightProgram, m_pointLightProgram, m_proceduralProgram; //Shader Program Data
	unsigned int m_lastKey[2];
public:
	std::vector<PointLight> m_pointLights;
	std::vector<DirectionalLight> m_dirLights;

	VirtualWorld();
	virtual ~VirtualWorld();

	virtual bool startup();
	virtual bool shutdown();

	virtual bool update();
	virtual void draw();
	virtual void resize(int a_width, int a_height);

	void BuildFrameBuffers();
	void BuildQuad();
	void BuildCube();
	void BuildProceduralGrid(vec2 a_realDims, glm::ivec2 a_dims);
	void BuildPerlinTexture(glm::ivec2 a_dims, const int a_octaves, const float a_amplitude, const float a_persistance);

	void RenderDirectionalLights();
	void RenderPointLights();

	void ReloadShaders();

	void AddDirectionalLight(vec3 a_dir = vec3(0, -1, 0), vec3 a_color = vec3(1));
	void AddPointLight(vec3 a_pos = vec3(0), vec3 a_color = vec3(1), float a_radius = 25.0f);

	void AddFBXModel( const char* a_filename, vec3 a_pos = vec3(0), float a_roughness = 0.3f, float a_fresnelScale = 2.0f, 
					  mat4 a_modTransform = mat4(1), vec3 m_scale = vec3(1), quat m_rot = quat::tquat());

	void AddPhysModel(const char* a_filename, vec3 a_pos, PxGeometry* a_geometry, PxMaterial* a_physicsMaterial, 
					  float a_density = 10.0f, float a_roughness = 0.3f, float a_fresnelScale = 2.0f, 
					  mat4 a_modTransform = mat4(1), vec3 m_scale = vec3(1), quat m_rot = quat::tquat());

	void AddParticleEmitter(vec3 a_pos = vec3(0), vec3 a_extents = vec3(1), unsigned int a_maxParticles = 100,
							float a_lifespanMin = 1.0f, float a_lifespanMax = 2.0f, float a_velocityMin = 1.0f, 
							float a_velocityMax = 2.0f, float a_fadeIn = 0.0f, float a_fadeOut = 0.0f,
							float a_startSize = 1.0f, float a_endSize = 0.5f, vec4 a_startColor = vec4(1), vec4 a_endColor = vec4(1),
							EmitType a_emitType = EmitType(0), MoveType a_moveType = MoveType(0), char* a_szFilename = "./data/textures/white.png");
};

#endif//VIRTUAL_WORLD_H_