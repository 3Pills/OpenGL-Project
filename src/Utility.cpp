#include <iostream>
#include "gl_core_4_4.h"
#include <GLFW\glfw3.h>
#include "glm_header.h"
#include "AntTweakBar.h"

unsigned int LoadShader(char* a_vertex_Filename, char* a_fragment_Filename, GLuint* result){
	bool succeeded = false;
	FILE* vs_file = fopen(a_vertex_Filename, "rb");
	FILE* fs_file = fopen(a_fragment_Filename, "rb");

	if (vs_file == 0){
		printf("%s", "Vertex shader not found!");
	}
	else if (fs_file == 0){
		printf("%s", "Fragment shader not found!");
	}
	else {
		fseek(vs_file, 0, SEEK_END);
		long vs_file_len = ftell(vs_file);
		rewind(vs_file);

		fseek(fs_file, 0, SEEK_END);
		long fs_file_len = ftell(fs_file);
		rewind(fs_file);

		char* vs_source = new char[vs_file_len+1];
		char* fs_source = new char[fs_file_len+1];

		fread(vs_source, 1, vs_file_len, vs_file);
		fread(fs_source, 1, fs_file_len, fs_file);

		vs_source[vs_file_len] = '\0';
		fs_source[fs_file_len] = '\0';

		succeeded = true;

		unsigned int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
		unsigned int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

		glShaderSource(vertex_shader, 1, &vs_source, 0);
		glCompileShader(vertex_shader);

		glShaderSource(fragment_shader, 1, &fs_source, 0);
		glCompileShader(fragment_shader);

		*result = glCreateProgram();
		glAttachShader(*result, vertex_shader);
		glAttachShader(*result, fragment_shader);
		glLinkProgram(*result);

		int success = GL_FALSE;
		glGetProgramiv(*result, GL_LINK_STATUS, &success);
		if (success == GL_FALSE){
			int log_length = 0;
			glGetProgramiv(*result, GL_INFO_LOG_LENGTH, &log_length);

			char* log = new char[log_length];
			glGetProgramInfoLog(*result, log_length, 0, log);
			printf("Error: Failed to link shader program!\n");
			printf("%s\n", log);
			delete[] log;
			succeeded = false;
		}

		glDeleteShader(fragment_shader);
		glDeleteShader(vertex_shader);

		delete[] vs_source;
		delete[] fs_source;
		fclose(vs_file);
		fclose(fs_file);
	}

	return succeeded;
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