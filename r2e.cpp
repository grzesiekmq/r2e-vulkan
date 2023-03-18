// r2e.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include <vector>
#include "renderer.h"




const std::vector<const char*> validationLayers = {  };




renderer r;

void cleanup() {

}

void mainLoop() {

}

void init() {

}


int main()
{
	r.windowInit();
	r.init();
	r.update();
	r.cleanup();



	return 0;
}


