#ifndef PROCEDURAL_GENERATION_H_
#define PROCEDURAL_GENERATION_H_
#include "Application.h"
#include "Camera.h"
#include "Vertex.h"

class ProceduralGeneration : public Application
{
	FlyCamera m_oCamera;

	OpenGLData m_planeMesh;
	unsigned int m_programID;
	unsigned int m_perlinTexture;
	float* m_perlinData;

	int m_pOct;
	float m_pAmp, m_pPers, m_pScale;
public:
	ProceduralGeneration();
	virtual ~ProceduralGeneration();

	virtual bool startup();
	virtual bool shutdown();

	virtual bool update();
	virtual void draw();

	void BuildGrid(vec2 a_realDims, glm::ivec2 a_dims);
	void BuildPerlinTexture(glm::ivec2 a_dims, const int a_octaves, const float a_amplitude, const float a_persistance);

	void PerlinNoise(vec2 a_coords);

	void ReloadShader();
};

#endif//PROCEDURAL_GENERATION_H_