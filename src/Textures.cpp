#include "Textures.h"
#include "stb_image.h"
#include "Vertex.h"
#include "Utility.h"

Textures::Textures():m_oCamera(50){
	Application::Application();
}
Textures::~Textures(){}

bool Textures::startup(){
	if (!Application::startup()){
		return false;
	}

	glEnable(GL_DEPTH_TEST);
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);

	m_oCamera.setPerspective(glm::radians(50.0f), 1280.0f / 720.0f, 0.1f, 20000.0f);
	LoadTexture("./textures/crate.png", 0);
	LoadTexture("./textures/planets/earth_cloud.jpg", 1);
	GenerateQuad(5.0f);
	LoadShader("./shaders/textures_vertex.glsl", "", "./shaders/textures_fragment.glsl", &m_programID);

	Gizmos::create();

	return true;
}
bool Textures::shutdown(){
	return Application::shutdown();
}
bool Textures::update(){
	if (!Application::update()){
		return false;
	}

	m_oCamera.update(m_fDeltaTime);
	return true;
}
void Textures::draw(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(m_programID);

	int view_proj_uniform = glGetUniformLocation(m_programID, "projView");
	if (view_proj_uniform > -1) {
		glUniformMatrix4fv(view_proj_uniform, 1, GL_FALSE, (float*)&m_oCamera.getProjectionView());
	}

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_Texture[0]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_Texture[1]);

	int diffuse_location = glGetUniformLocation(m_programID, "diffuse");
	if (diffuse_location > -1){
		glUniform1i(diffuse_location, 0);
	}
	int normal_location = glGetUniformLocation(m_programID, "normal");
	if (normal_location > -1){
		glUniform1i(normal_location, 1);
	}

	glBindVertexArray(m_VAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

	Gizmos::addTransform(mat4(1), 10);
	Gizmos::draw(m_oCamera.getProjectionView());
	Application::draw();
}

void Textures::LoadTexture(const char* a_szFileName, const int a_TextureID){
	int width, height, channels;
	unsigned char* data = stbi_load(a_szFileName, &width, &height, &channels, STBI_default);

	glGenTextures(1, &m_Texture[a_TextureID]);
	glBindTexture(GL_TEXTURE_2D, m_Texture[a_TextureID]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	stbi_image_free(data);
}

void Textures::GenerateQuad(const float a_fSize){
	VertexTexCoord vertex_data[4];
	vertex_data[0].position = vec4(-a_fSize, 0, -a_fSize, 1);
	vertex_data[1].position = vec4(-a_fSize, 0,  a_fSize, 1);
	vertex_data[2].position = vec4( a_fSize, 0,  a_fSize, 1);
	vertex_data[3].position = vec4( a_fSize, 0, -a_fSize, 1);

	vertex_data[0].tex_coord = vec2(0, 0);
	vertex_data[1].tex_coord = vec2(0, 1);
	vertex_data[2].tex_coord = vec2(1, 1);
	vertex_data[3].tex_coord = vec2(1, 0);

	unsigned int index_data[6] = { 0, 2, 1, 0, 3, 2 };

	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO);
	glGenBuffers(1, &m_IBO);
	glBindVertexArray(m_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(VertexTexCoord) * 4, vertex_data, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 6, index_data, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(VertexTexCoord), 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexTexCoord), (void*)sizeof(vec4));
	
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}