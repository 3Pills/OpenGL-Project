#ifndef _UTILITY_H_
#define _UTILITY_H_
#include "gl_core_4_4.h"
#include <GLFW\glfw3.h>
#include "Vertex.h"	

typedef unsigned int GLuint;
bool LoadShader(const char* a_filename, GLenum a_shaderType, unsigned int* a_output);
bool LoadShader(const char* a_vertexFileName, const char* a_geometryFileName, const char* a_fragmentFileName, GLuint* a_result);
void OnMouseButton(GLFWwindow* window, int button, int pressed, int altKeys);
void OnMousePosition(GLFWwindow* window, double x, double y);
void OnMouseScroll(GLFWwindow* window, double x, double y);
void OnKey(GLFWwindow* window, int key, int scanCode, int pressed, int modKeys);
void OnChar(GLFWwindow* window, unsigned int c);
void OnWindowResize(GLFWwindow* window, int width, int height);
void RenderPlane(const vec4 a_plane);

OpenGLData LoadOBJ(const char* filename);

#endif//_UTILITY_H_