#ifndef _UTILITY_H_
#define _UTILITY_H_

typedef unsigned int GLuint;
unsigned int LoadShader(char* a_vertex_Filename, char* a_fragment_Filename, GLuint* result);
void OnMouseButton(GLFWwindow* window, int button, int pressed, int altKeys);
void OnMousePosition(GLFWwindow* window, double x, double y);
void OnMouseScroll(GLFWwindow* window, double x, double y);
void OnKey(GLFWwindow* window, int key, int scanCode, int pressed, int modKeys);
void OnChar(GLFWwindow* window, unsigned int c);
void OnWindowResize(GLFWwindow* window, int width, int height);

#endif//_UTILITY_H_