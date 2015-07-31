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

struct ClothData {
	PxCloth* m_cloth;

	vec3* m_vertices;
	vec2* m_texCoords;
	unsigned int* m_indices;

	unsigned int m_texture;
	unsigned int m_indexCount, m_vertexCount;
	unsigned int m_VAO, m_VBO, m_textureVBO, m_IBO;

	ClothData(const unsigned int a_springSize, const unsigned int a_rows, const unsigned int a_columns) {
		// this position will represent the top middle vertex
		glm::vec3 clothPosition = glm::vec3(0, 12, 0);
		// shifting grid position for looks
		float halfWidth = a_rows * a_springSize * 0.5f;
		// generate vertices for a grid with texture coordinates
		m_vertexCount = a_rows * a_columns;
		m_vertices = new glm::vec3[m_vertexCount];
		m_texCoords = new glm::vec2[m_vertexCount];
		for (unsigned int r = 0; r < a_rows; ++r) {
			for (unsigned int c = 0; c < a_columns; ++c) {
				m_vertices[r * a_columns + c].x = clothPosition.x + a_springSize * c;
				m_vertices[r * a_columns + c].y = clothPosition.y;
				m_vertices[r * a_columns + c].z = clothPosition.z + a_springSize * r - halfWidth;
				m_texCoords[r * a_columns + c].x = 1.0f - r / (a_rows - 1.0f);
				m_texCoords[r * a_columns + c].y = 1.0f - c / (a_columns - 1.0f);
			}
		}

		m_indexCount = (a_rows - 1) * (a_columns - 1) * 2 * 3;
		m_indices = new unsigned int[m_indexCount];
		unsigned int* index = m_indices;
		for (unsigned int r = 0; r < (a_rows - 1); ++r) {
			for (unsigned int c = 0; c < (a_columns - 1); ++c) {
				// indices for the 4 quad corner vertices
				unsigned int i0 = r * a_columns + c;
				unsigned int i1 = i0 + 1;
				unsigned int i2 = i0 + a_columns;
				unsigned int i3 = i2 + 1;
				// every second quad changes the triangle order
				if ((c + r) % 2) {
					*index++ = i0; *index++ = i2; *index++ = i1;
					*index++ = i1; *index++ = i2; *index++ = i3;
				}
				else {
					*index++ = i0; *index++ = i2; *index++ = i3;
					*index++ = i0; *index++ = i3; *index++ = i1;
				}
			}
		}

	}
	~ClothData() {
		delete[] m_vertices;
		delete[] m_indices;
		delete[] m_texCoords;
	}

	void GLGenBuffers() {
		glGenVertexArrays(1, &m_VAO);
		glGenBuffers(1, &m_VBO);
		glGenBuffers(1, &m_IBO);

		glBindVertexArray(m_VAO);
		glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO);

		glBufferData(GL_ARRAY_BUFFER, (sizeof(vec3) + sizeof(vec2)) * m_vertexCount, 0, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec3) * m_vertexCount, m_vertices);
		glBufferSubData(GL_ARRAY_BUFFER, sizeof(vec3)* m_vertexCount, sizeof(vec2)* m_vertexCount, m_texCoords);

		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)* m_indexCount, m_indices, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		//glEnableVertexAttribArray(2);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(vec3)* m_vertexCount));
		//glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, 0);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	void draw() {
		glBindVertexArray(m_VAO);
		glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, 0);
	}
};

class VirtualWorld : public Application
{
	FlyCamera m_oCamera;
	std::vector<GPUEmitter*> m_particleEmitters;
	std::vector<FBXModel*> m_FBXModels;

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
	std::vector<ClothData*> m_cloths;

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

	void AddFBXModel(FBXModel* a_model);
	void AddParticleEmitter(GPUEmitter* a_particle);

	void AddCloth(PxCloth* a_cloth);
};

#endif//VIRTUAL_WORLD_H_