#ifndef LIGHTS_H_
#define LIGHTS_H_
#include "Utility.h"
#include "Camera.h"

//CameraDirection struct for rendering pointlight shadow maps.
struct CameraDirection {
	GLenum CubemapFace;
	vec3 Target;
	vec3 Up;
};

//Point light data storage.
struct PointLight {
	vec3 m_pos;
	vec3 m_color;
	float m_radius;
	const static mat4 m_lightProj;
	const static CameraDirection m_cubemapDirection[6];

	unsigned int m_shadowFBO; //Shadow buffer framebuffer object
	unsigned int m_shadowMap; //Shadow buffer texture data
	unsigned int m_depthTexture; //Shadow buffer texture data

	PointLight(const vec3 a_pos, const vec3 a_color, const float a_radius);
	~PointLight();

	void Render(Camera* a_camera, const unsigned int a_program);
};

//Directional light data storage.
struct DirectionalLight {
	vec3 m_dir;
	vec3 m_color;
	const static mat4 m_lightProj;

	unsigned int m_shadowFBO; //Shadow buffer framebuffer object
	unsigned int m_shadowMap; //Shadow buffer texture data

	DirectionalLight(const vec3 a_dir, const vec3 a_color);
	~DirectionalLight();

	void Render(Camera* a_camera, const unsigned int a_program);
};

class Sprite {
	float m_timer; //Global sprite timer

	OpenGLData m_buffers; //OpenGL buffers
	VertexParticle m_vertexData[4]; //Vertex data for OpenGL buffers
	unsigned int m_indexData[6]; //Index data for OpenGL buffers

	unsigned int m_texture; //Texture handle for sprite texture
	unsigned int m_shader;
public:
	vec3 m_pos; //Sprite position
	vec3 m_color, m_maxColor, m_minColor; //Color values
	float m_size, m_maxSize, m_minSize; //Size values
	float m_frequency; //Frequency for any size/color updates

	Sprite(const vec3 a_pos, const float a_size, const vec3 a_color, const char* a_filename);
	Sprite(const vec3 a_pos, const float a_maxSize, const float a_minSize, const vec3 a_maxColor, const vec3 a_minColor, const char* a_filename);
	~Sprite();

	void Init(const vec3 a_pos, const float a_maxSize, const float a_minSize, const vec3 a_maxColor, const vec3 a_minColor, const char* a_filename);
	void Update(const float dt, const mat4 a_camTransform);
	void Render(Camera* a_camera);
};

#endif//LIGHTS_H_