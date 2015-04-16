#ifndef _UTILITY_H_
#define _UTILITY_H_
#include "gl_core_4_4.h"
#include <GLFW\glfw3.h>

typedef unsigned int GLuint;
bool LoadShader(char* a_filename, GLenum a_shaderType, unsigned int* a_output);
bool LoadShader(char* a_vertexFileName, char* a_geometryFileName, char* a_fragmentFileName, GLuint* a_result);
void OnMouseButton(GLFWwindow* window, int button, int pressed, int altKeys);
void OnMousePosition(GLFWwindow* window, double x, double y);
void OnMouseScroll(GLFWwindow* window, double x, double y);
void OnKey(GLFWwindow* window, int key, int scanCode, int pressed, int modKeys);
void OnChar(GLFWwindow* window, unsigned int c);
void OnWindowResize(GLFWwindow* window, int width, int height);
void RenderPlane(vec4 a_plane);

OpenGLData LoadOBJ(const char* filename);

typedef enum EmitType {
	EMIT_POINT = 0,
	EMIT_LINE = 1,
	EMIT_PLANE = 2,
	EMIT_RECTANGLE = 3,
	EMIT_OUTER_RECTANGLE = 4,
	EMIT_RING = 5,
	EMIT_OUTER_RING = 6,
	EMIT_SPHERE = 7,
	EMIT_OUTER_SPHERE = 8,
	EMIT_COUNT = 9
};

#endif//_UTILITY_H_