#include "AdvancedTextures.h"
//#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Utility.h"

AdvancedTextures::AdvancedTextures() : m_oCamera(50), m_vAmbCol(vec3(0.4f)), m_vLightCol(vec3(0.85f)), m_vLightPos(vec3(0, 10, 0)), m_fSpecPow(16),
									   m_vBgCol(vec4(0.3f,0.3f,0.3f,1)) {
	Application::Application();
}
AdvancedTextures::~AdvancedTextures(){}

bool AdvancedTextures::startup(){
	if (!Application::startup()){
		return false;
	}
	glEnable(GL_DEPTH_TEST);

	TwInit(TW_OPENGL_CORE, nullptr);
	TwWindowSize(1280, 720);

	glfwSetMouseButtonCallback(m_window, OnMouseButton);
	glfwSetCursorPosCallback(m_window, OnMousePosition);
	glfwSetScrollCallback(m_window, OnMouseScroll);

	glfwSetKeyCallback(m_window, OnKey);
	glfwSetCharCallback(m_window, OnChar);

	glfwSetWindowSizeCallback(m_window, OnWindowResize);

	LoadTextures();
	GenerateQuad(5.0f);
	LoadShader("./shaders/normal_mapped_vertex.glsl", "", "./shaders/normal_mapped_fragment.glsl", &m_programID);

	m_oCamera.setPerspective(glm::radians(50.0f), 1280.0f / 720.0f, 0.1f, 20000.0f);
	Gizmos::create();
	
	TwBar* m_bar = TwNewBar("Mars Bar");
	TwAddVarRW(m_bar, "Clear Color", TW_TYPE_COLOR4F, &m_vBgCol, "");
	TwAddVarRW(m_bar, "Direction", TW_TYPE_DIR3F, &m_vLightPos, "group=Light");
	TwAddVarRW(m_bar, "Color", TW_TYPE_COLOR3F, &m_vLightCol, "group=Light");
	TwAddVarRW(m_bar, "Specular Power", TW_TYPE_FLOAT, &m_fSpecPow, "group=Light min=0.05 step=0.05 max=1000.0");

	TwAddVarRO(m_bar, "FPS", TW_TYPE_FLOAT, &m_fFPS, "");

	return true;
}
bool AdvancedTextures::shutdown(){
	Gizmos::destroy();
	TwTerminate();
	return Application::shutdown();
}
bool AdvancedTextures::update(){
	if (!Application::update()){
		return false;
	}
	m_fFPS = 1 / m_fDeltaTime;
	glClearColor(m_vBgCol.x, m_vBgCol.y, m_vBgCol.z, m_vBgCol.w);
	if (glfwGetKey(m_window, GLFW_KEY_R) == GLFW_PRESS && m_LastKey != GLFW_PRESS){
		ReloadShader();
	}
	m_LastKey = glfwGetKey(m_window, GLFW_KEY_R);

	vec3 forward = (vec3)m_oCamera.getWorldTransform()[2];
	forward.y = 0;
	forward = glm::normalize(forward);
	vec3 side = (vec3)m_oCamera.getWorldTransform()[0];

	if (glfwGetKey(m_window, GLFW_KEY_KP_2) == GLFW_PRESS){
		m_vLightPos += forward * 20 * m_fDeltaTime;
	}
	if (glfwGetKey(m_window, GLFW_KEY_KP_8) == GLFW_PRESS){
		m_vLightPos -= forward * 20 * m_fDeltaTime;
	}
	if (glfwGetKey(m_window, GLFW_KEY_KP_4) == GLFW_PRESS){
		m_vLightPos -= side * 20 * m_fDeltaTime;
	}
	if (glfwGetKey(m_window, GLFW_KEY_KP_6) == GLFW_PRESS){
		m_vLightPos += side * 20 * m_fDeltaTime;
	}
	if (glfwGetKey(m_window, GLFW_KEY_KP_7) == GLFW_PRESS){
		m_vLightPos.y -= 20 * m_fDeltaTime;
	}
	if (glfwGetKey(m_window, GLFW_KEY_KP_9) == GLFW_PRESS){
		m_vLightPos.y += 20 * m_fDeltaTime;
	}

	m_oCamera.update(m_fDeltaTime);
	return true;
}
void AdvancedTextures::draw(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	Gizmos::clear();

	glUseProgram(m_programID);

	int view_proj_uniform = glGetUniformLocation(m_programID, "ProjectionView");
	if (view_proj_uniform > -1) {
		glUniformMatrix4fv(view_proj_uniform, 1, GL_FALSE, (float*)&m_oCamera.getProjectionView());
	}
	int amb_color_uniform = glGetUniformLocation(m_programID, "ambientColor");
	if (amb_color_uniform > -1) {
		glUniform3fv(amb_color_uniform, 1, (float*)&m_vAmbCol);
	}
	int dif_color_uniform = glGetUniformLocation(m_programID, "lightColor");
	if (dif_color_uniform > -1) {
		glUniform3fv(dif_color_uniform, 1, (float*)&m_vLightCol);
	}
	int light_dir_uniform = glGetUniformLocation(m_programID, "lightDir");
	if (light_dir_uniform > -1) {
		glUniform3fv(light_dir_uniform, 1, (float*)&(glm::normalize(-m_vLightPos)));
	}
	int cam_pos_uniform = glGetUniformLocation(m_programID, "cameraPos");
	if (cam_pos_uniform > -1) {
		glUniform3fv(cam_pos_uniform, 1, (float*)&m_oCamera.getWorldTransform()[3].xyz);
	}
	int spec_pow_uniform = glGetUniformLocation(m_programID, "specPow");
	if (spec_pow_uniform > -1) {
		glUniform1f(spec_pow_uniform, m_fSpecPow);
	}

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_diffTex);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_normTex);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_specTex);

	int diff_location = glGetUniformLocation(m_programID, "diffTex");
	if (diff_location > -1){
		glUniform1i(diff_location, 0);
	}
	int norm_location = glGetUniformLocation(m_programID, "normTex");
	if (norm_location > -1){
		glUniform1i(norm_location, 1);
	}
	int spec_location = glGetUniformLocation(m_programID, "specTex");
	if (spec_location > -1){
		glUniform1i(spec_location, 2);
	}

	glBindVertexArray(m_glData.m_VAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

	mat4 transformPos = mat4(1);
	transformPos[3].xyz = m_vLightPos;
	Gizmos::addTransform(transformPos, 1);

	vec4 white(1);
	vec4 black(0, 0, 0, 1);

	for (int i = 0; i <= 20; ++i){
		Gizmos::addLine(vec3(-10 + i, 0, -10), i != 10 ? vec3(-10 + i, 0, 10) : vec3(-10 + i, 0, 0), i != 10 ? black : white);
		Gizmos::addLine(vec3(-10, 0, -10 + i), i != 10 ? vec3(10, 0, -10 + i) : vec3(0, 0, -10 + i), i != 10 ? black : white);
	}

	Gizmos::draw(m_oCamera.getProjectionView());
	TwDraw();
	Application::draw();
}

void AdvancedTextures::LoadTextures(){
	int width, height, channels;
	unsigned char* data = stbi_load("./textures/rock_diffuse.tga", &width, &height, &channels, STBI_default);

	glGenTextures(1, &m_diffTex);
	glBindTexture(GL_TEXTURE_2D, m_diffTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	stbi_image_free(data);

	data = stbi_load("./textures/rock_normal.tga", &width, &height, &channels, STBI_default);

	glGenTextures(1, &m_normTex);
	glBindTexture(GL_TEXTURE_2D, m_normTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	stbi_image_free(data);

	data = stbi_load("./textures/rock_specular.tga", &width, &height, &channels, STBI_default);

	glGenTextures(1, &m_specTex);
	glBindTexture(GL_TEXTURE_2D, m_specTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	stbi_image_free(data);
}

void AdvancedTextures::GenerateQuad(const float a_fSize){
	VertexNormal vertex_data[4];
	vertex_data[0].position = vec4(-a_fSize, 0, -a_fSize, 1);
	vertex_data[1].position = vec4(-a_fSize, 0, a_fSize, 1);
	vertex_data[2].position = vec4(a_fSize, 0, a_fSize, 1);
	vertex_data[3].position = vec4(a_fSize, 0, -a_fSize, 1);

	vertex_data[0].normal = vec4(0, 1, 0, 0);
	vertex_data[1].normal = vec4(0, 1, 0, 0);
	vertex_data[2].normal = vec4(0, 1, 0, 0);
	vertex_data[3].normal = vec4(0, 1, 0, 0);

	vertex_data[0].tangent = vec4(1, 0, 0, 0);
	vertex_data[1].tangent = vec4(1, 0, 0, 0);
	vertex_data[2].tangent = vec4(1, 0, 0, 0);
	vertex_data[3].tangent = vec4(1, 0, 0, 0);

	vertex_data[0].tex_coord = vec2(0, 0);
	vertex_data[1].tex_coord = vec2(0, 1);
	vertex_data[2].tex_coord = vec2(1, 1);
	vertex_data[3].tex_coord = vec2(1, 0);

	unsigned int index_data[6] = { 0, 2, 1, 0, 3, 2 };
	m_glData.m_indexCount = 6;

	glGenVertexArrays(1, &m_glData.m_VAO);
	glGenBuffers(1, &m_glData.m_VBO);
	glGenBuffers(1, &m_glData.m_IBO);
	glBindVertexArray(m_glData.m_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, m_glData.m_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(VertexNormal)* 4, vertex_data, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_glData.m_IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)* 6, index_data, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);//position
	glEnableVertexAttribArray(1);//normal
	glEnableVertexAttribArray(2);//tangent
	glEnableVertexAttribArray(3);//tex_coord

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(VertexNormal), 0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_TRUE,  sizeof(VertexNormal), (void*)(sizeof(vec4)));
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_TRUE,  sizeof(VertexNormal), (void*)(sizeof(vec4) * 2));
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(VertexNormal), (void*)(sizeof(vec4) * 3));

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void AdvancedTextures::ReloadShader(){
	glDeleteProgram(m_programID);
	LoadShader("./shaders/normal_mapped_vertex.glsl", "", "./shaders/normal_mapped_fragment.glsl", &m_programID);
}