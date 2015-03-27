#include "ProceduralGeneration.h"
#include "Utility.h"
#include "AntTweakBar.h"

ProceduralGeneration::ProceduralGeneration(): m_oCamera(50), m_pOct(6), m_pAmp(1.0f), m_pPers(0.3f), m_pScale(5.0f){
	Application::Application();
}

ProceduralGeneration::~ProceduralGeneration(){}

bool ProceduralGeneration::startup(){
	if (!Application::startup()){
		return false;
	}

	TwInit(TW_OPENGL_CORE, nullptr);
	TwWindowSize(1280, 720);

	glfwSetMouseButtonCallback(m_window, OnMouseButton);
	glfwSetCursorPosCallback(m_window, OnMousePosition);
	glfwSetScrollCallback(m_window, OnMouseScroll);

	glfwSetKeyCallback(m_window, OnKey);
	glfwSetCharCallback(m_window, OnChar);

	glfwSetWindowSizeCallback(m_window, OnWindowResize);

	m_oCamera.setPerspective(glm::radians(50.0f), 1280.0f / 720.0f, 0.1f, 20000.0f);

	glm::ivec2 dims = glm::ivec2(256, 256);
	BuildGrid(vec2(40, 40), dims);
	BuildPerlinTexture(dims, m_pOct, m_pAmp, m_pPers);

	LoadShader("./shaders/perlin_vertex.glsl", 0, "./shaders/perlin_fragment.glsl", &m_programID);

	TwBar* m_bar = TwNewBar("Settings");
	TwAddVarRW(m_bar, "Scale", TW_TYPE_FLOAT, &m_pScale, "step=0.05");
	TwAddVarRW(m_bar, "Octaves", TW_TYPE_INT32, &m_pOct, "");
	TwAddVarRW(m_bar, "Amplitude", TW_TYPE_FLOAT, &m_pAmp, "step=0.01");
	TwAddVarRW(m_bar, "Persistance", TW_TYPE_FLOAT, &m_pPers, "step=0.01");

	Gizmos::create();
	return true;
}

bool ProceduralGeneration::shutdown(){
	Gizmos::destroy();
	return Application::shutdown();
}

bool ProceduralGeneration::update(){
	if (!Application::update()){
		return false;
	}
	m_oCamera.update(m_fDeltaTime);
	Gizmos::clear();
	Gizmos::addTransform(mat4(1), 10);

	vec4 white(1);
	vec4 black(0, 0, 0, 1);

	for (int i = 0; i <= 20; ++i){
		Gizmos::addLine(vec3(-10 + i, 0, -10), i != 10 ? vec3(-10 + i, 0, 10) : vec3(-10 + i, 0, 0), i != 10 ? black : white);
		Gizmos::addLine(vec3(-10, 0, -10 + i), i != 10 ? vec3(10, 0, -10 + i) : vec3(0, 0, -10 + i), i != 10 ? black : white);
	}
	if (glfwGetKey(m_window, GLFW_KEY_R)){
		ReloadShader();
	}
	if (glfwGetKey(m_window, GLFW_KEY_T)){
		BuildPerlinTexture(glm::ivec2(64, 64), m_pOct, m_pAmp, m_pPers);
	}

	return true;
}

void ProceduralGeneration::draw(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(m_programID);

	int projView_uniform = glGetUniformLocation(m_programID, "projView");
	int perlinTexture_uniform = glGetUniformLocation(m_programID, "perlinTexture");
	int scale_uniform = glGetUniformLocation(m_programID, "scale");

	glUniformMatrix4fv(projView_uniform, 1, GL_FALSE, (float*)&m_oCamera.getProjectionView());
	glUniform1f(scale_uniform, m_pScale);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_perlinTexture);
	glUniform1i(perlinTexture_uniform, 0);

	glBindVertexArray(m_planeMesh.m_VAO);
	glDrawElements(GL_TRIANGLES, m_planeMesh.m_indexCount, GL_UNSIGNED_INT, 0);

	TwDraw();
	Gizmos::draw(m_oCamera.getProjectionView());
	Application::draw();
}

void ProceduralGeneration::BuildGrid(vec2 a_realDims, glm::ivec2 a_dims){
	int vertexCount = (a_dims.x + 1) * (a_dims.y + 1);

	VertexTexCoord* vertexData = new VertexTexCoord[vertexCount];

	unsigned int indexCount = a_dims.x * a_dims.y * 6;
	unsigned int* indexData = new unsigned int[indexCount];

	float currY = -a_realDims.y / 2.0f;
	for (int y = 0; y < a_dims.y + 1; ++y){
		float currX = -a_realDims.x / 2.0f;
		for (int x = 0; x < a_dims.x + 1; ++x){
			vertexData[y * (a_dims.x + 1) + x].position = vec4(currX, 0, currY, 1);
			vertexData[y * (a_dims.x + 1) + x].tex_coord = vec2((float)x / (float)a_dims.x, (float)y / (float)a_dims.y);
			currX += a_realDims.x / (float)a_dims.x;
		}
		currY += a_realDims.y / (float)a_dims.y;
	}

	int currIndex = 0;
	for (int y = 0; y < a_dims.y; ++y){
		for (int x = 0; x < a_dims.x; ++x){
			indexData[currIndex++] =  y		 * (a_dims.x + 1) +  x;
			indexData[currIndex++] = (y + 1) * (a_dims.x + 1) +  x;
			indexData[currIndex++] = (y + 1) * (a_dims.x + 1) + (x + 1);

			indexData[currIndex++] = (y + 1) * (a_dims.x + 1) + (x + 1);
			indexData[currIndex++] =  y		 * (a_dims.x + 1) + (x + 1);
			indexData[currIndex++] =  y		 * (a_dims.x + 1) +  x;
		}
	}

	m_planeMesh.m_indexCount = indexCount;

	glGenVertexArrays(1, &m_planeMesh.m_VAO);
	glGenBuffers(1, &m_planeMesh.m_VBO);
	glGenBuffers(1, &m_planeMesh.m_IBO);

	glBindVertexArray(m_planeMesh.m_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_planeMesh.m_VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_planeMesh.m_IBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(VertexTexCoord)* vertexCount, vertexData, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)* indexCount, indexData, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(VertexTexCoord), 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexTexCoord), (void*)sizeof(vec4));

	delete[] vertexData;
	delete[] indexData;
}

void ProceduralGeneration::BuildPerlinTexture(glm::ivec2 a_dims, const int a_octaves, const float a_amplitude, const float a_persistance) {
	float scale = (1.0f / a_dims.x) * 5.0f;

	m_perlinData = new float[a_dims.x * a_dims.y];
	
	for (int y = 0; y < a_dims.y; y++) {
		for (int x = 0; x < a_dims.x; x++) {

			float amplitude = a_amplitude;
			float freq = 1;

			m_perlinData[y*a_dims.x + x] = 0;

			for (int o = 0; o < a_octaves; o++) {
				float perlinSample = glm::perlin(vec2((float)x, (float)y) * scale * freq) * 0.5 + 0.5;

				perlinSample *= amplitude;
				m_perlinData[y*a_dims.x + x] += perlinSample;

				amplitude *= a_persistance;
				freq *= 2;
			}
		}
	}

	glGenTextures(1, &m_perlinTexture);
	glBindTexture(GL_TEXTURE_2D, m_perlinTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, a_dims.x, a_dims.y, 0, GL_RED, GL_FLOAT, m_perlinData);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void ProceduralGeneration::PerlinNoise(vec2 a_coords){
	vec2 clampedCoords = vec2(fmod(a_coords.x, 1), fmod(a_coords.y, 1));
	vec2 corners[4] = { vec2(0, 0), vec2(0, 1), vec2(1, 1), vec2(1, 0) };

	srand(0);
	vec2 g1 = glm::normalize(vec2(rand(), rand()));
	vec2 g2 = glm::normalize(vec2(rand(), rand()));
	vec2 g3 = glm::normalize(vec2(rand(), rand()));
	vec2 g4 = glm::normalize(vec2(rand(), rand()));

	vec2 d1 = clampedCoords - corners[0];
	vec2 d2 = clampedCoords - corners[1];
	vec2 d3 = clampedCoords - corners[2];
	vec2 d4 = clampedCoords - corners[3];

	float i1 = glm::dot(g1, d1);
	float i2 = glm::dot(g2, d2);
	float i3 = glm::dot(g3, d3);
	float i4 = glm::dot(g4, d4);
	
	float l1 = glm::lerp(i1, i2, clampedCoords.x);
	float l2 = glm::lerp(i3, i4, clampedCoords.x);

	float average = glm::lerp(l1, l2, clampedCoords.y);
}

void ProceduralGeneration::ReloadShader(){
	glDeleteProgram(m_programID);
	LoadShader("./shaders/perlin_vertex.glsl", 0, "./shaders/perlin_fragment.glsl", &m_programID);
}