#include "RenderingGeometry.h"
#include "Vertex.h"
#include "Utility.h"

RenderingGeometry::RenderingGeometry() : m_oCamera(50){
	Application::Application();
}
RenderingGeometry::~RenderingGeometry(){}

bool RenderingGeometry::startup(){
	if (!Application::startup()){
		return false;
	}

	if (!LoadShader("shaders/vertex.glsl", "shaders/geometry_fragment.glsl", &m_programID)){
		return false;
	}
	generateGrid(100,100);

	Gizmos::create();
	m_oCamera.setPerspective(glm::radians(50.0f), 1280.0f / 720.0f, 0.1f, 20000.0f);

	return true;
}
bool RenderingGeometry::shutdown(){
	return Application::shutdown();
}
bool RenderingGeometry::update(){
	if (!Application::update()){
		return false;
	}
	m_oCamera.update(m_fDeltaTime);
	return true;
}
void RenderingGeometry::draw(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	Gizmos::clear();
	glUseProgram(m_programID);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	int proj_view_handle = glGetUniformLocation(m_programID, "ProjectionView");
	if (proj_view_handle > -1){
		glUniformMatrix4fv(proj_view_handle, 1, false, (float*)&m_oCamera.getProjectionView());
	}
	int time_handle = glGetUniformLocation(m_programID, "time");
	if (time_handle > -1){
		glUniform1f(time_handle, m_fCurrTime);
	}
	int heightScale_handle = glGetUniformLocation(m_programID, "heightScale");
	if (heightScale_handle > -1){
		glUniform1f(heightScale_handle, 10);
	}

	glBindVertexArray(m_VAO);
	glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, 0);
	//glDrawElements(GL_LINES, m_indexCount, GL_UNSIGNED_INT, 0);

	vec4 white(1);
	vec4 black(0, 0, 0, 1);

	for (int i = 0; i <= 20; ++i){
		Gizmos::addLine(vec3(-10 + i, 0, -10), i != 10 ? vec3(-10 + i, 0, 10) : vec3(-10 + i, 0, 0), i != 10 ? black : white);
		Gizmos::addLine(vec3(-10, 0, -10 + i), i != 10 ? vec3(10, 0, -10 + i) : vec3(0, 0, -10 + i), i != 10 ? black : white);
	}

	Gizmos::addTransform(mat4(1), 10);
	Gizmos::draw(m_oCamera.getProjectionView());

	Application::draw();
}

void RenderingGeometry::generateGrid(const unsigned int rows, const unsigned int cols) {
	Vertex* vertex_array = new Vertex[(rows + 1) * (cols + 1)];

	for (unsigned int r = 0; r < rows + 1; ++r) {
		for (unsigned int c = 0; c < cols + 1; ++c) {
			vec4 pos = vec4((float)c, 0, (float)r, 1);
			vertex_array[c + r * (cols + 1)].position = pos;

			vec4 col = vec4((float)c / (cols + 1), 0, (float)r / (rows + 1), 1);
			vertex_array[c + r * (cols + 1)].color = col;
		}
	}

	m_indexCount = rows * cols * 6;
	unsigned int* index_array = new unsigned int[m_indexCount];
	int index_location = 0;

	for (unsigned int r = 0; r < rows; ++r) {
		for (unsigned int c = 0; c < cols; ++c) {
			index_array[index_location++] = c + r * (cols + 1);
			index_array[index_location++] = c + (r + 1) * (cols + 1);
			index_array[index_location++] = (c + 1) + r * (cols + 1);

			index_array[index_location++] = index_array[index_location - 1];
			index_array[index_location++] = index_array[index_location - 3];
			index_array[index_location++] = (c + 1) + (r + 1) * (cols + 1);
		}
	}

	glGenBuffers(1, &m_VBO);
	glGenBuffers(1, &m_IBO);
	glGenVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, (cols + 1)*(rows + 1)*sizeof(Vertex), vertex_array, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0); //position
	glEnableVertexAttribArray(1); //color

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)sizeof(vec4));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indexCount * sizeof(unsigned int), index_array, GL_STATIC_DRAW);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	delete[] vertex_array;
	delete[] index_array;
}