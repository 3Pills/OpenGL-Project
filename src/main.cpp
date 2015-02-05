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
	glEnable(GL_DEPTH_TEST);

	Gizmos::create();

	mat4 view = glm::lookAt(vec3(10, 10, 10), vec3(0, 0, 0), vec3(0, 1, 0));
	mat4 proj = glm::perspective(glm::radians(60.0f), 1280.0f / 720.0f, 0.1f, 1000.0f);

	vec3 pos = vec3(10, 10, 10);
	float timer = 0.0f;

	glfwSetTime(0.0);
	while (!glfwWindowShouldClose(window)){
		float dt = (float)glfwGetTime();
		glfwSetTime(0.0);
		timer += dt;

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		Gizmos::clear();
		Gizmos::addTransform( mat4(1), 10);

		view = glm::lookAt(pos, vec3(0, 0, 0), vec3(0, 1, 0));
		//pos.x = sinf(timer) * 10;
		//pos.z = cosf(timer) * 10;

		vec4 white(1);
		vec4 black(0, 0, 0, 1);
		vec4 red(1, 0, 0, 1);
		vec4 green(0, 1, 0, 1);
		vec4 blue(0, 0, 1, 1);

		for (int i = 0; i <= 20; ++i){
			Gizmos::addLine(vec3(-10 + i, 0, -10), vec3(-10 + i, 0, 10), i == 10 ? white : black);
			Gizmos::addLine(vec3(-10, 0, -10 + i), vec3(10, 0, -10 + i), i == 10 ? white : black);
		}
		Gizmos::addTri(vec3(0, 0, 0), vec3(0, 0, 5), vec3(0, 5, 0), green);
		Gizmos::addAABBFilled(vec3(0, 4, -5), vec3(1,1,1), blue);
		Gizmos::draw(proj, view);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}