#include "glm_header.h"
#include "AntTweakBar.h"
#include "gl_core_4_4.h"
#include <GLFW\glfw3.h>
#include "Gizmos.h"

bool LoadShader(char* a_filename, GLenum a_shaderType, unsigned int* a_output) {
	bool succeeded = true;

	FILE* shaderFile = fopen(a_filename, "r");

	//did it open successfully
	if (shaderFile == 0)
		return false;
	else {
		// find out how long the file is
		fseek(shaderFile, 0, SEEK_END);
		int shaderFileLen = ftell(shaderFile);
		fseek(shaderFile, 0, SEEK_SET);

		//allocate enough space for the shader
		char *shaderSource = new char[shaderFileLen];
		//read the file anmd update the length to be accurate
		shaderFileLen = fread(shaderSource, 1, shaderFileLen, shaderFile);


		int logLength = 0;

		//create the shader handle
		unsigned int shaderHandle = glCreateShader(a_shaderType);
		//compile the shader
		glShaderSource(shaderHandle, 1, &shaderSource, &shaderFileLen);
		glCompileShader(shaderHandle);
		//catch for errors
		int success = GL_FALSE;
		glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &success);
		if (success == GL_FALSE)
		{
			glGetShaderiv(shaderHandle, GL_INFO_LOG_LENGTH, &logLength);
			char* log = new char[logLength];
			glGetShaderInfoLog(shaderHandle, logLength, NULL, log);
			printf("%s\n", log);
			delete[] log;
			succeeded = false;
		}
		if (succeeded)
			*a_output = shaderHandle;

		delete[] shaderSource;
		fclose(shaderFile);
	}
	return succeeded;
}

bool LoadShader(char* a_vertexFileName, char* a_geometryFileName, char* a_fragmentFileName, GLuint* a_result) {
	bool succeeded = true;

	*a_result = glCreateProgram();

	if (a_vertexFileName != nullptr) {
		unsigned int vertexShader;
		LoadShader(a_vertexFileName, GL_VERTEX_SHADER, &vertexShader);
		glAttachShader(*a_result, vertexShader);
		glDeleteShader(vertexShader);
	}

	if (a_geometryFileName != nullptr) {
		unsigned int geometryShader;
		LoadShader(a_geometryFileName, GL_GEOMETRY_SHADER, &geometryShader);
		glAttachShader(*a_result, geometryShader);
		glDeleteShader(geometryShader);
	}

	if (a_fragmentFileName != nullptr) {
		unsigned int fragmentShader;
		LoadShader(a_fragmentFileName, GL_FRAGMENT_SHADER, &fragmentShader);
		glAttachShader(*a_result, fragmentShader);
		glDeleteShader(fragmentShader);
	}

	glLinkProgram(*a_result);

	GLint success;

	glGetProgramiv(*a_result, GL_LINK_STATUS, &success);

	if (success == GL_FALSE)
	{
		GLint log_length;
		glGetProgramiv(*a_result, GL_INFO_LOG_LENGTH, &log_length);

		char* log = new char[log_length];
		glGetProgramInfoLog(*a_result, log_length, 0, log);

		printf("Error: Failed to link shader program!\n");
		printf("%s\n", log);

		delete[] log;
		succeeded = false;
	}
	return succeeded;
}

void RenderPlane(vec4 a_plane) {
	vec3 up = vec3(0, 1, 0);
	if (a_plane.xyz() == vec3(0, 1, 0))
	{
		up = vec3(1, 0, 0);
	}

	vec3 tangent = glm::normalize(glm::cross(a_plane.xyz(), up));
	vec3 bitangent = glm::normalize(glm::cross(a_plane.xyz(), tangent));

	vec3 p = a_plane.xyz() * a_plane.w;

	vec3 v0 = p + tangent + bitangent;
	vec3 v1 = p + tangent - bitangent;
	vec3 v2 = p - tangent - bitangent;
	vec3 v3 = p - tangent + bitangent;

	Gizmos::addTri(v0, v1, v2, vec4(1, 1, 0, 1));
	Gizmos::addTri(v0, v2, v3, vec4(1, 1, 0, 1));

	//Gizmos::addLine(p, p + a_plane.xyz() * 2, vec4(0, 1, 1, 1));
}

void OnMouseButton(GLFWwindow* window, int button, int pressed, int altKeys) {
	TwEventMouseButtonGLFW(button, pressed);
}

void OnMousePosition(GLFWwindow* window, double x, double y) {
	TwEventMousePosGLFW((int)x, (int)y);
}

void OnMouseScroll(GLFWwindow* window, double x, double y) {
	TwEventMouseWheelGLFW((int)y);
}

void OnKey(GLFWwindow* window, int key, int scanCode, int pressed, int modKeys) {
	TwEventKeyGLFW(key, pressed);
}

void OnChar(GLFWwindow* window, unsigned int c) {
	TwEventCharGLFW(c, GLFW_PRESS);
}

void OnWindowResize(GLFWwindow* window, int width, int height) {
	TwWindowSize(width, height);
	glViewport(0, 0, width, height);
}