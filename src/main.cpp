#include "gl_core_4_4.h"
#include <GLFW\glfw3.h>
#include "Gizmos.h"
#include <iostream>

#define GLM_SWIZZLE

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

	vec3 pos = vec3(1000, 5, 0);
	vec3 look = vec3(0, 5, 0);
	mat4 view = glm::lookAt(pos, look, vec3(0, 1, 0));
	mat4 proj = glm::perspective(glm::radians(1.0f), 1280.0f / 720.0f, 0.1f, 20000.0f);

	float timer = 0.0f;

	glfwSetTime(0.0);
	while (!glfwWindowShouldClose(window)){
		float dt = (float)glfwGetTime();
		glfwSetTime(0.0);
		timer += dt;

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		Gizmos::clear();
		Gizmos::addTransform( mat4(1), 10);

		view = glm::lookAt(pos, look, vec3(0, 1, 0));
		//pos.x = sinf(timer) * 10;
		//pos.z = cosf(timer) * 10;

		vec4 white(1);
		vec4 black(0, 0, 0, 1);
		vec4 red(0.5, 0, 0, 1);
		vec4 yellow(1, 1, 0, 1);
		vec4 grey(0.5, 0.5, 0.2, 1);
		vec4 greytwo(0.25, 0.25, 0.1, 1);
		vec4 green(0, 1, 0, 1);
		vec4 blue(0, 0, 1, 1);

		for (int i = 0; i <= 20; ++i){
			Gizmos::addLine(vec3(-10 + i, 0, -10), vec3(-10 + i, 0, 10), i == 10 ? white : black);
			Gizmos::addLine(vec3(-10, 0, -10 + i), vec3(10, 0, -10 + i), i == 10 ? white : black);
		}
		mat4 sun_transform = glm::rotate(timer, vec3(0, 1, 0));
		mat4 child_one_transform = sun_transform * glm::translate(vec3(4.5, 0, 0)) * glm::rotate(timer * 2.f, vec3(0, 1, 0));
		mat4 child_two_transform = child_one_transform * glm::translate(vec3(0, 0, 0));

		Gizmos::addSphere(sun_transform[3].xyz, 4, 16, 16, blue, &sun_transform);
		Gizmos::addSphere(child_two_transform[3].xyz, 1, 16, 16, black, &child_two_transform);

		Gizmos::addTri(vec3(0, 9, 0), vec3(0, 10, -2), vec3(0, 10, 0), green);
		//Gizmos::addAABBFilled(vec3(0, 4, -5), vec3(1,1,1), blue);

		//Gizmos::addAABBFilled(vec3(0, 0.5, 0.5), vec3(0.5, 0.5, 0.5), greytwo);
		//Gizmos::addAABBFilled(vec3(0, 1, 2.5), vec3(0.5, 1, 1.5), greytwo);
		//Gizmos::addAABBFilled(vec3(0, 3, 3.5), vec3(0.5, 1, 1.5), red);
		//Gizmos::addAABBFilled(vec3(0, 3, 8.5), vec3(0.5, 1, 1.5), red);
		//Gizmos::addAABBFilled(vec3(0, 1, 9.5), vec3(0.5, 1, 1.5), greytwo);
		//Gizmos::addAABBFilled(vec3(0, 0.5, 11.5), vec3(0.5, 0.5, 0.5), greytwo);

		Gizmos::draw(proj, view);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}