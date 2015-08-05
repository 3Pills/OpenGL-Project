#include "Lighting.h"
#include "Utility.h"

Lighting::Lighting() :m_oCamera(50), m_vAmbCol(vec3(0.4f)), m_vMatCol(vec3(1)), m_vLightCol(vec3(0.5f)), m_vLightPos(vec3(0, 10, 0)), m_fSpecPow(16) {
	Application::Application();
}
Lighting::~Lighting(){}

bool Lighting::startup(){
	if (!Application::startup()){
		return false;
	}

	glEnable(GL_DEPTH_TEST);
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);

	m_oCamera.SetPerspective(glm::radians(50.0f), 1280.0f / 720.0f, 0.1f, 20000.0f);
	Gizmos::create();

	LoadShader("./data/shaders/lighting_vertex.glsl", "", "./data/shaders/lighting_fragment.glsl", &m_programID);
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string err = tinyobj::LoadObj(shapes, materials, "./data/models/stanford/bunny.obj");

	if (err.size() > 0){
		printf("%s", err);
		system("pause");
		return false;
	}

	CreateOpenGLBuffers(shapes);
	return true;
}
bool Lighting::shutdown(){
	CleanOpenGLBuffers();
	Gizmos::destroy();
	glDeleteProgram(m_programID);
	return Application::shutdown();
}
bool Lighting::update(){
	if (!Application::update()){
		return false;
	}
	if (glfwGetKey(m_window, GLFW_KEY_R) == GLFW_PRESS && m_LastKey != GLFW_PRESS){
		ReloadShader();
	}
	m_LastKey = glfwGetKey(m_window, GLFW_KEY_R);

	vec3 forward = (vec3)m_oCamera.GetWorldTransform()[2];
	forward.y = 0;
	forward = glm::normalize(forward);
	vec3 side = (vec3)m_oCamera.GetWorldTransform()[0];

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

	if (glfwGetKey(m_window, GLFW_KEY_KP_1) == GLFW_PRESS){
		if (m_fSpecPow > 2){
			m_fSpecPow /= 2;
		}
	}
	if (glfwGetKey(m_window, GLFW_KEY_KP_3) == GLFW_PRESS){
		m_fSpecPow *= 2;
	}

	if (glfwGetKey(m_window, GLFW_KEY_KP_5) == GLFW_PRESS){
		m_vLightPos = vec3(0, 10, 0);
		m_vLightCol = vec3(0.5f);
		m_vAmbCol = vec3(0.35f);
		m_fSpecPow = 16;
	}

	if (glfwGetKey(m_window, GLFW_KEY_UP) == GLFW_PRESS){
		m_vLightCol = vec3(0.5f);
		m_vAmbCol = vec3(0.35f);
	}
	else if (glfwGetKey(m_window, GLFW_KEY_LEFT) == GLFW_PRESS){
		m_vLightCol = vec3(0.5f, 0, 0);
		m_vAmbCol = vec3(0.35f, 0, 0);
	}
	else if(glfwGetKey(m_window, GLFW_KEY_DOWN) == GLFW_PRESS){
		m_vLightCol = vec3(0, 0.5f, 0);
		m_vAmbCol = vec3(0, 0.35f, 0);
	}
	else if(glfwGetKey(m_window, GLFW_KEY_RIGHT) == GLFW_PRESS){
		m_vLightCol = vec3(0, 0, 0.5f);
		m_vAmbCol = vec3(0, 0, 0.35f);
	}

	m_oCamera.Update(m_fDeltaTime);
	return true;
}
void Lighting::draw(){
	Gizmos::clear();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(m_programID);

	int view_proj_uniform = glGetUniformLocation(m_programID, "projView");
	if (view_proj_uniform > -1) {
		glUniformMatrix4fv(view_proj_uniform, 1, GL_FALSE, (float*)&m_oCamera.GetProjectionView());
	}
	int amb_color_uniform = glGetUniformLocation(m_programID, "ambCol");
	if (amb_color_uniform > -1) {
		glUniform3fv(amb_color_uniform, 1, (float*)&m_vAmbCol);
	}
	int mat_color_uniform = glGetUniformLocation(m_programID, "matCol");
	if (mat_color_uniform > -1) {
		glUniform3fv(mat_color_uniform, 1, (float*)&m_vMatCol);
	}
	int dif_color_uniform = glGetUniformLocation(m_programID, "lightCol");
	if (dif_color_uniform > -1) {
		glUniform3fv(dif_color_uniform, 1, (float*)&m_vLightCol);
	}
	int light_dir_uniform = glGetUniformLocation(m_programID, "lightDir");
	if (light_dir_uniform > -1) {
		glUniform3fv(light_dir_uniform, 1, (float*)&(glm::normalize(-m_vLightPos)));
	}
	int cam_pos_uniform = glGetUniformLocation(m_programID, "camPos");
	if (cam_pos_uniform > -1) {
		glUniform3fv(cam_pos_uniform, 1, (float*)&m_oCamera.GetWorldTransform()[3].xyz);
	}
	int spec_pow_uniform = glGetUniformLocation(m_programID, "specPow");
	if (spec_pow_uniform > -1) {
		glUniform1f(spec_pow_uniform, m_fSpecPow);
	}

	for (unsigned int i = 0; i < m_glData.size(); ++i){
		glBindVertexArray(m_glData[i].m_VAO);
		glDrawElements(GL_TRIANGLES, m_glData[i].m_indexCount, GL_UNSIGNED_INT, 0);
	}

	vec4 white(1);
	vec4 black(0, 0, 0, 1);
	for (int i = 0; i <= 20; ++i){
		Gizmos::addLine(vec3(-10 + i, 0, -10), vec3(-10 + i, 0, 10), i != 10 ? black : white);
		Gizmos::addLine(vec3(-10, 0, -10 + i), vec3(10, 0, -10 + i), i != 10 ? black : white);
	}

	mat4 transformPos = mat4(1);
	transformPos[3].xyz = m_vLightPos;
	Gizmos::addTransform(transformPos, 1);
	Gizmos::draw(m_oCamera.GetProjectionView());
	Application::draw();
}

void Lighting::CreateOpenGLBuffers(std::vector<tinyobj::shape_t>& shapes){
	m_glData.resize(shapes.size());

	for (unsigned int shape_index = 0; shape_index < shapes.size(); ++shape_index){
		std::vector<float> vertex_data;

		unsigned int floatCount = shapes[shape_index].mesh.positions.size();
		floatCount += shapes[shape_index].mesh.normals.size();

		vertex_data.reserve(floatCount);
		vertex_data.insert(vertex_data.end(), shapes[shape_index].mesh.positions.begin(), shapes[shape_index].mesh.positions.end());
		vertex_data.insert(vertex_data.end(), shapes[shape_index].mesh.normals.begin(),   shapes[shape_index].mesh.normals.end());

		m_glData[shape_index].m_indexCount = shapes[shape_index].mesh.indices.size();

		glGenVertexArrays(1, &m_glData[shape_index].m_VAO);
		glGenBuffers(1, &m_glData[shape_index].m_VBO);
		glGenBuffers(1, &m_glData[shape_index].m_IBO);

		glBindVertexArray(m_glData[shape_index].m_VAO);
		glBindBuffer(GL_ARRAY_BUFFER, m_glData[shape_index].m_VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * floatCount, vertex_data.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_glData[shape_index].m_IBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, shapes[shape_index].mesh.indices.size() * sizeof(unsigned int), shapes[shape_index].mesh.indices.data(), GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);//position
		glEnableVertexAttribArray(1);//normal data

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, 0, (void*)(sizeof(float) * shapes[shape_index].mesh.positions.size()));

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
}

void Lighting::CleanOpenGLBuffers(){
	glDeleteProgram(m_programID);
	for (unsigned int i = 0; i < m_glData.size(); ++i){
		glDeleteVertexArrays(1, &m_glData[i].m_VAO);
		glDeleteBuffers(1, &m_glData[i].m_VBO);
		glDeleteBuffers(1, &m_glData[i].m_IBO);
	}
}

void Lighting::ReloadShader(){
	glDeleteProgram(m_programID);
	LoadShader("./data/shaders/lighting_vertex.glsl", "", "./data/shaders/lighting_fragment.glsl", &m_programID);
}