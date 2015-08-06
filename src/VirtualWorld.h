#ifndef VIRTUAL_WORLD_H_
#define VIRTUAL_WORLD_H_
#include "Application.h"
#include "Camera.h"
#include <vector>
#include "GPUEmitter.h"
#include "FBXModel.h"
#include "PhysModel.h"
#include "PhysScene.h"

//Point light data storage.
struct PointLight {
	vec3 m_pos;
	vec3 m_color;
	float m_radius;

	PointLight(vec3 a_pos, vec3 a_color, float a_radius) : m_pos(a_pos), m_color(a_color), m_radius(a_radius) {}
};

//Directional light data storage.
struct DirectionalLight {
	vec3 m_dir;
	vec3 m_color;

	DirectionalLight(vec3 a_dir, vec3 a_color) : m_dir(glm::normalize(a_dir)), m_color(a_color) {}
};

class VirtualWorld : public Application
{
	TargetCamera m_oCamera;
	std::vector<GPUEmitter*> m_particleEmitters;
	std::vector<FBXModel*> m_FBXModels;

	OpenGLData m_screenspaceQuad; //Quad which deferred render will render onto
	OpenGLData m_lightCube; //Cube for point lights
	OpenGLData m_planeMesh; //Mesh for perlin heightmap

	PhysScene m_physScene; //PhysX scene

	PxController* m_player; //Player controller for scene
	vec3 m_playerVelocity; //Player's velocity
	float m_playerSpeed; //Player's speed
	bool m_playerGrounded; //Bool for grounded player

	vec3 m_ambCol; //Global ambient colour for lighting

	float m_pHeightScale; //Perlin landscape scale
	float m_pAmp; //Perlin landscape amplitude
	float m_pPers; //Perlin landscape persistence
	unsigned int m_pOct; //Perlin landscape octaves

	float m_pRoughness, m_pFresnelScale; //Perlin lighting value

	float* m_pData; //Perlin heightmap data
	unsigned int m_pHeightTexture; //Procedural heightmap data
	float m_pTextureScale; //Texture UV scale on perlin mesh
	unsigned int m_pTexture; //Procedural diffuse texture data

	vec2 m_pTextureSize; //Perlin texture size
	vec2 m_pWorldSize; //Perlin mesh size (X/Z Axis)
	vec3 m_pPos; //Position transform for perlin mesh
	vec3 m_pScale; //Scale transform for perlin mesh
	quat m_pRot; //Rotation transform for perlin mesh
	mat4 m_pTransform; //Final transform for perlin mesh

	bool m_debug[10]; //Debug rendering bools. 0:Z-Buffer, 1:Grid, 2:PhysX, 3:Particles, 4:Point Lights, 5:Directional Lights

	unsigned int m_shadowFBO; //Shadow buffer framebuffer object
	unsigned int m_shadowMap; //Shadow buffer texture data

	unsigned int m_gBufferFBO; //G-buffer framebuffer object
	unsigned int m_albedoTexture, m_positionTexture, m_normalTexture, m_specularTexture, m_depthTexture; //G-Buffer texture data

	unsigned int m_lightFBO; //Light buffer framebuffer object
	unsigned int m_lightTexture; //Light buffer texture data

	unsigned int m_fxFBO; //Effects framebuffer object
	unsigned int m_fxTexture; //Effects buffer data

	unsigned int m_gBufferProgram, m_compositeProgram, m_dirLightProgram, m_pointLightProgram, m_proceduralProgram; //Shader Program Data
	unsigned int m_lastKey[2]; //Input key logging
public:
	std::vector<PointLight> m_pointLights; //Point light storage array
	std::vector<DirectionalLight> m_dirLights; //Directional light storage array
	std::vector<ClothData*> m_cloths; //Cloth storage array

	VirtualWorld();
	virtual ~VirtualWorld();

	virtual bool startup();
	virtual bool shutdown();

	virtual bool update();
	virtual void draw();
	virtual void resize(int a_width, int a_height);

	void ReloadShaders();

	void BuildFrameBuffers();
	void BuildQuad();
	void BuildCube();

	void BuildProceduralGrid(vec2 a_realDims, glm::ivec2 a_dims);
	void BuildPerlinTexture(glm::ivec2 a_dims, const int a_octaves, const float a_amplitude, const float a_persistance);

	void AddDirectionalLight(vec3 a_dir = vec3(0, -1, 0), vec3 a_color = vec3(1));
	void AddPointLight(vec3 a_pos = vec3(0), vec3 a_color = vec3(1), float a_radius = 25.0f);

	void RenderDirectionalLights();
	void RenderPointLights();

	void AddFBXModel(FBXModel* a_model);
	void AddParticleEmitter(GPUEmitter* a_particle);
	void AddCloth(PxCloth* a_cloth);

	void PlayerUpdate();
};

#endif//VIRTUAL_WORLD_H_