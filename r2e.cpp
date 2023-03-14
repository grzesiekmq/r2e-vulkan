// r2e.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include <vector>
#include <renderer.h>




const std::vector<const char*> validationLayers = {  };

const uint32_t width = 800;
const uint32_t height = 600;

GLFWwindow* window;

renderer r;


void cleanup() {
	
}

void mainLoop() {
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		r.render();

	}
	r.device->waitIdle();
}

void init() {
	
}


int main()
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	window = glfwCreateWindow(width, height, "Vulkan triangle", nullptr, nullptr);


	mainLoop();


	glfwDestroyWindow(window);

	glfwTerminate();

	return 0;
}


