#pragma once
#include <GLFW/glfw3.h>

using namespace vk;
 
struct renderer {
	GLFWwindow* window;

	UniqueInstance instance;
	PhysicalDevice gpu;
	UniqueDevice device;
	Queue gfxQueue;
	Queue presentQueue;
	SurfaceKHR surface;
	SwapchainKHR swapchain;
	RenderPass rp;
	std::vector<Image> images;
	std::vector<ImageView> imageViews;
	Extent2D extent  ;
	std::vector<Framebuffer> framebuffers;
 	Pipeline pipeline;
	PipelineLayout pipelineLayout;
	CommandPool commandPool;
	std::vector<CommandBuffer> commandBuffers;
 	ImageSubresourceRange imgRange;

	Semaphore imgSemaphore;
	Semaphore renderSemaphore;

	Fence fence;
public:
	bool createInstance();
	bool createSurface();
	bool selectGpu();
	bool createDevice();

	bool createSwapchain();
	bool createImageViews();
	bool createRenderPass();
	bool createPipeline();
	bool createFramebuffers();
	bool createCommandPool();
	bool createCommandBuffers();
	ShaderModule createShaderModule(const std::vector<char>& code);
	bool createSemaphores();
	bool createFence();
	void render();
	std::vector<char>   readSpv(const std::string filename);
	 renderer();
	 ~renderer();

} ;






