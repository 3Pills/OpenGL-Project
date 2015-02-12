#ifndef _TEXTURES_H_
#define _TEXTURES_H_
#include "Application.h"
#include "Camera.h"

class Textures : public Application {
	FlyCamera m_oCamera;
public:
	Textures();
	virtual ~Textures();

	virtual bool startup();
	virtual bool shutdown();

	virtual bool update();
	virtual void draw();

	void LoadTexture(const char* a_szFileName, const int a_TextureID);
	void GenerateQuad(const float a_fSize);

	unsigned int m_Texture[4];
	unsigned int m_programID;

	unsigned int m_VAO;
	unsigned int m_VBO;
	unsigned int m_IBO;
};

#endif//_TEXTURES_H_