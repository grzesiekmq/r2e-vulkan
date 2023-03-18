#pragma once
#include <GLFW/glfw3.h>
#include <vector>

using namespace vk;
const uint32_t width = 800;
const uint32_t height = 600;


struct renderer {
	GLFWwindow* window;
	Extent2D extent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

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
	void init();
	void cleanup();
	void update();
	void windowInit();
	void createVertexBuffer();
	void loadModel();
} ;






