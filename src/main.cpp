#include "gl_core_4_4.h"
#define GLM_SWIZZLE

#include <GLFW\glfw3.h>
#include <iostream>

#include "Gizmos.h"
#include "glm/glm.hpp"
#include "glm/ext.hpp"

using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat4;

int main() {
	if (!glfwInit()){ 
		return -1; 
	}

	GLFWwindow* window = glfwCreateWindow(1280, 720, "Untitled Window", nullptr, nullptr);
	if (window == nullptr) { 
		return -2; 
	}

	glfwMakeContextCurrent(window);
	if (ogl_LoadFunctions() == ogl_LOAD_FAILED){
		glfwDestroyWindow(window);
		glfwTerminate();
		return -3;
	}

	int major_version = ogl_GetMajorVersion();
	int minor_version = ogl_GetMinorVersion();

	printf("OpenGL V%d.%d successfully loaded.", major_version, minor_version);

	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);

	while (!glfwWindowShouldClose(window)){
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}