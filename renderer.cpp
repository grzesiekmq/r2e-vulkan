#define VULKAN_HPP_NO_CONSTRUCTORS
#define TINYOBJLOADER_IMPLEMENTATION

#define GLM_ENABLE_EXPERIMENTAL

#include "tiny_obj_loader.h"
#include <glm/glm.hpp>
#include <optional>
#include <vector>
#include <vulkan/vulkan.hpp>
#include <iostream>
#include <fstream>
#include <array>
#include "renderer.h"


using namespace vk;

Buffer vb;

DeviceMemory vbMem;

const std::vector<const char*> deviceExt = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
struct QueueFamilyIndices {
	std::optional<uint32_t> gfxFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete() {
		return gfxFamily.has_value() && presentFamily.has_value();
	}
};
QueueFamilyIndices findQueueFamilies(PhysicalDevice device, SurfaceKHR surface) {
	QueueFamilyIndices indices;
	auto queueFamilies = device.getQueueFamilyProperties();

	int i = 0;
	for (const auto& queueFamily : queueFamilies) {
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & QueueFlagBits::eGraphics) {
			indices.gfxFamily = i;
		}
		if (queueFamily.queueCount > 0 && device.getSurfaceSupportKHR(i, surface)) {
			indices.presentFamily = i;
		}
		if (indices.isComplete()) {
			break;
		}
		i++;
	}
	return indices;
}
uint32_t findMemType(PhysicalDevice gpu, uint32_t typeFilter, MemoryPropertyFlags
	properties) {
	PhysicalDeviceMemoryProperties memProperties;
	memProperties =  gpu.getMemoryProperties();

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

}
struct Vertex {
	glm::vec2 pos;
};

std::vector<Vertex> vertices;
VertexInputBindingDescription bindingDesc{
	.binding = 0,
	.stride = sizeof(Vertex),
	.inputRate = VertexInputRate::eVertex
};

std::array<VertexInputAttributeDescription, 1> attribDesc{

attribDesc[0].binding = 0,
attribDesc[0].location = 0,
attribDesc[0].format = Format::eR32G32Sfloat,
attribDesc[0].offset = offsetof(Vertex, pos)
};

void renderer::init() {
	createInstance();
	createSurface();
	selectGpu();
	createDevice();

	createSwapchain();
	createImageViews();
	createRenderPass();
	createPipeline();
	createFramebuffers();
	loadModel();
	createCommandPool();
	createVertexBuffer();
	createCommandBuffers();

	createSemaphores();
	createFence();
}
void renderer::cleanup() {
	device->destroyFence(fence);
	device->destroySemaphore(renderSemaphore);
	device->destroySemaphore(imgSemaphore);


	device->freeCommandBuffers(commandPool, commandBuffers);

	device->destroyCommandPool(commandPool);

	for (auto& framebuffer : framebuffers) {
		device->destroyFramebuffer(framebuffer);
	}

	device->destroyPipeline(pipeline);
	device->destroyPipelineLayout(pipelineLayout);
	device->destroyRenderPass(rp);

	for (auto& imageView : imageViews) {
		device->destroyImageView(imageView);
	}

	device->destroySwapchainKHR(swapchain);
	device->destroyBuffer(vb);
	device->freeMemory(vbMem);
	instance->destroySurfaceKHR(surface);

	glfwDestroyWindow(window);

	glfwTerminate();
}
void renderer::update() {
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		 render();

	}
	 device->waitIdle();
}

void renderer::windowInit() {
	glfwInit();
	
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	window = glfwCreateWindow(width, height, "Vulkan triangle", nullptr, nullptr);

}

bool renderer::createInstance() {
	uint32_t glfwExtCount = 0;
	const char** glfwExt;

	glfwExt = glfwGetRequiredInstanceExtensions(&glfwExtCount);
	std::vector<const char*> ext(glfwExt, glfwExt
		+ glfwExtCount);

	ApplicationInfo info{
		.pApplicationName = "Triangle",
		.applicationVersion = VK_MAKE_VERSION(1,0,0),
		.pEngineName = "r2e",
		.engineVersion = VK_MAKE_VERSION(1,0,0),
		.apiVersion = VK_API_VERSION_1_3
	};

	InstanceCreateInfo ci{
		.pApplicationInfo = &info,
		.enabledExtensionCount = glfwExtCount,
		.ppEnabledExtensionNames = ext.data()

	};
	instance = createInstanceUnique(ci);

	std::cout << "instance created" << std::endl;

	return true;
}

bool renderer::selectGpu() {
	auto gpus = instance->enumeratePhysicalDevices();

	gpu = gpus.front();

	std::cout << "gpu selected" << std::endl;
	return true;
}





bool renderer::createDevice() {
	auto features = PhysicalDeviceFeatures();
	float priority = 1.0f;
	DeviceQueueCreateInfo ci{
		.queueFamilyIndex = 0,
		.queueCount = 1,
		.pQueuePriorities = &priority
	};
	DeviceCreateInfo deviceCi{
		.queueCreateInfoCount = 1,
		.pQueueCreateInfos = &ci,
		.enabledExtensionCount = static_cast<uint32_t>(deviceExt.size()),
		.ppEnabledExtensionNames = deviceExt.data(),
		.pEnabledFeatures = &features
	};

	device = gpu.createDeviceUnique(deviceCi);

	uint32_t queueGfxFamilyIndex = 0;

	

	

	gfxQueue = device->getQueue(findQueueFamilies(gpu, surface).gfxFamily.value(), 0);
	presentQueue = device->getQueue(findQueueFamilies(gpu, surface).presentFamily.value(), 0);
	std::cout << "device created" << std::endl;
	return true;
}

bool renderer::createSurface() {
	VkSurfaceKHR surf;
	VkResult result;
 	result = glfwCreateWindowSurface((VkInstance)*instance, window, nullptr, &surf);

	std::cout << "result is " << result << std::endl;

	surface = static_cast<SurfaceKHR>(surf);
	std::cout << "surface created" << std::endl;
	return true;
}

bool renderer::createSwapchain() {
	SurfaceCapabilitiesKHR caps;

	caps = gpu.getSurfaceCapabilitiesKHR(surface);

	SwapchainCreateInfoKHR ci{ .surface = surface,
	.minImageCount = caps.minImageCount,
	.imageFormat = Format::eB8G8R8A8Srgb,
	.imageColorSpace = ColorSpaceKHR::eSrgbNonlinear,
	.imageExtent = caps.currentExtent,
	.imageArrayLayers = 1,
	.imageUsage = ImageUsageFlagBits::eColorAttachment,
	.imageSharingMode = SharingMode::eExclusive,
	.presentMode = PresentModeKHR::eFifo,
	.clipped = true,
	.oldSwapchain = SwapchainKHR(nullptr) };

	swapchain = device->createSwapchainKHR(ci);

	images = device->getSwapchainImagesKHR(swapchain);

	std::cout << "swapchain created" << std::endl;

	return true;
}

bool renderer::createImageViews() {
	imageViews.resize(images.size());

	for (size_t i = 0; i < images.size(); i++) {
		ImageViewCreateInfo ci{
			.image = images[i],
			.viewType = ImageViewType::e2D,
			.format = Format::eB8G8R8A8Srgb,
			.components = {.r = ComponentSwizzle::eIdentity,
		.g = ComponentSwizzle::eIdentity,
		.b = ComponentSwizzle::eIdentity,
		.a = ComponentSwizzle::eIdentity},
		.subresourceRange = {.aspectMask = ImageAspectFlagBits::eColor,
		.baseMipLevel = 0,
		.levelCount = 1,
		.baseArrayLayer = 0,
		.layerCount = 1}
		};

		imageViews[i] = device->createImageView(ci);

	}
	std::cout << "image views created" << std::endl;

	return true;
}

ShaderModule renderer::createShaderModule(const std::vector<char>& code) {

	ShaderModuleCreateInfo ci{ .codeSize = code.size(),
	.pCode = reinterpret_cast<const uint32_t*>(code.data()) };

	ShaderModule shaderModule;
	shaderModule = device->createShaderModule(ci);

	std::cout << "shader module created" << std::endl;

	return shaderModule;
}

bool renderer::createRenderPass() {

	AttachmentDescription colorAttachment{ .format = Format::eB8G8R8A8Srgb,
	.samples = SampleCountFlagBits::e1,
	.loadOp = AttachmentLoadOp::eClear,
	.storeOp = AttachmentStoreOp::eStore,
	.stencilLoadOp = AttachmentLoadOp::eDontCare,
	.stencilStoreOp = AttachmentStoreOp::eDontCare,
	.initialLayout = ImageLayout::eUndefined,
	.finalLayout = ImageLayout::ePresentSrcKHR };

	AttachmentReference colorAttachmentRef{ .attachment = 0,
	.layout = ImageLayout::eColorAttachmentOptimal };

	SubpassDescription subpass{ .pipelineBindPoint = PipelineBindPoint::eGraphics,
	.colorAttachmentCount = 1,
	.pColorAttachments = &colorAttachmentRef };

	SubpassDependency dependency{
		.srcSubpass = VK_SUBPASS_EXTERNAL,
		.dstSubpass = 0,
		.srcStageMask = PipelineStageFlagBits::eColorAttachmentOutput,
		.dstStageMask = PipelineStageFlagBits::eColorAttachmentOutput,
		.dstAccessMask = AccessFlagBits::eColorAttachmentWrite
	};

	PipelineLayout pipelineLayout;

	RenderPassCreateInfo ci{
		.attachmentCount = 1,
		.pAttachments = &colorAttachment,
		.subpassCount = 1,
		.pSubpasses = &subpass,
		.dependencyCount = 1,
		.pDependencies = &dependency
	};

	rp = device->createRenderPass(ci);

	std::cout << "render pass created" << std::endl;

	return true;
}

std::vector<char> renderer::readSpv(const std::string filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	size_t fileSize = static_cast<size_t>(file.tellg());
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();
	std::cout << "SPV INFO --------------------" << std::endl;
	std::cout << "buffer of shader file size is: " << fileSize << " and filename: " << filename << std::endl;

	return buffer;


}



void renderer::createVertexBuffer() {
	BufferCreateInfo bufferInfo{
		.size = sizeof(vertices[0]) * vertices.size(),
		.usage = BufferUsageFlagBits::eVertexBuffer,
		.sharingMode = SharingMode::eExclusive
	};

	vb = device->createBuffer(bufferInfo);

	MemoryRequirements memReq;

	memReq = device->getBufferMemoryRequirements(vb);

	MemoryAllocateInfo allocInfo{
		.allocationSize = memReq.size,
		.memoryTypeIndex = findMemType(gpu,memReq.memoryTypeBits, MemoryPropertyFlagBits::eHostVisible | MemoryPropertyFlagBits::eHostCoherent)

	};
	vbMem = device->allocateMemory(allocInfo);

	device->bindBufferMemory(vb, vbMem, 0);

	uint8_t* data = static_cast<uint8_t*>(device->mapMemory(vbMem, 0, memReq.size));
	memcpy(data, vertices.data(), static_cast<size_t>(bufferInfo.size));
	device->unmapMemory(vbMem);


}


void renderer::loadModel() {
	using namespace tinyobj;

	







		const std::string MODEL_PATH = "models/p1.obj";

		attrib_t attrib;
		std::vector<shape_t> shapes;
		// materials later
		LoadObj(&attrib, &shapes, nullptr, nullptr, nullptr, MODEL_PATH.c_str());

		for (const auto& shape : shapes) {
			for (const auto& index : shape.mesh.indices) {

				Vertex vertex{

				};
				vertex.pos = {
	   attrib.vertices[3 * index.vertex_index + 0],
	  attrib.vertices[3 * index.vertex_index + 1]

				};
				vertices.push_back(vertex);

			}
		}

	
}


bool renderer::createPipeline() {
	auto vertCode = readSpv("shaders/vert.spv");
	auto fragCode = readSpv("shaders/frag.spv");

	ShaderModule vertModule = createShaderModule(vertCode);
	ShaderModule fragModule = createShaderModule(fragCode);

	PipelineShaderStageCreateInfo vCi{ .stage = ShaderStageFlagBits::eVertex,
	.module = vertModule,
	.pName = "main" };

	PipelineShaderStageCreateInfo fCi{ .stage = ShaderStageFlagBits::eFragment,
	.module = fragModule,
	.pName = "main" };

	PipelineShaderStageCreateInfo shaderStages[] = { vCi, fCi };

	PipelineVertexInputStateCreateInfo vi{ .vertexBindingDescriptionCount = 1,
	.pVertexBindingDescriptions = &bindingDesc,
	.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribDesc.size()),
	.pVertexAttributeDescriptions = attribDesc.data()};


	PipelineInputAssemblyStateCreateInfo ia{ .topology = PrimitiveTopology::eTriangleList };

	Viewport vp{};
	vp.x = 		 0.0f;
	vp.width = (float)extent.width;
	vp.height = -(float)extent.height;
	vp.y = extent.height;
	vp.minDepth = 0.0f;
	vp.maxDepth = 1.0f;

	Rect2D scissor{};

	scissor.extent = extent;

	PipelineViewportStateCreateInfo viewport{
		.viewportCount = 1,
		.pViewports = &vp,
		.scissorCount = 1,
		.pScissors = &scissor
	};

	PipelineRasterizationStateCreateInfo rs{
		.polygonMode = PolygonMode::eFill,
		.cullMode = CullModeFlagBits::eBack,
		.frontFace = FrontFace::eClockwise,
		.lineWidth = 1.0f
	};

	ColorComponentFlags flags = ColorComponentFlagBits::eR | ColorComponentFlagBits::eG | ColorComponentFlagBits::eB
		| ColorComponentFlagBits::eA;

	PipelineColorBlendAttachmentState colorBlendAttachment{
		.blendEnable = VK_FALSE,
		.srcColorBlendFactor = BlendFactor::eZero,
		.dstColorBlendFactor = BlendFactor::eOne,
		.colorBlendOp = BlendOp::eAdd,
		.srcAlphaBlendFactor = BlendFactor::eZero,
		.dstAlphaBlendFactor = BlendFactor::eZero,
		.alphaBlendOp = BlendOp::eAdd,
		.colorWriteMask = flags
	};

	PipelineColorBlendStateCreateInfo cbs{
		.logicOp = LogicOp::eClear,
		.attachmentCount = 1,
		.pAttachments = &colorBlendAttachment

	};

	PipelineLayoutCreateInfo pipeLayoutCi{};

	pipelineLayout = device->createPipelineLayout(pipeLayoutCi);

	GraphicsPipelineCreateInfo pipelineCi{
		.stageCount = 2,
		.pStages = shaderStages,
		.pVertexInputState = &vi,
		.pInputAssemblyState = &ia,
		.pViewportState = &viewport,
		.pRasterizationState = &rs,
		.pColorBlendState = &cbs,
		.layout = pipelineLayout,
		.renderPass = rp
	};

	Result result;

	std::tie(result, pipeline) = device->createGraphicsPipeline(nullptr, pipelineCi);

	std::cout << "pipeline created" << std::endl;

	return true;


}

bool renderer::createFramebuffers() {
	framebuffers.resize(imageViews.size());

	for (size_t i = 0; i < imageViews.size(); i++) {
		ImageView attachments[] = {
			imageViews[i]
		};

		FramebufferCreateInfo ci{
			.renderPass = rp,
			.attachmentCount = 1,
			.pAttachments = attachments,
			.width = extent.width,
			.height = extent.height,
			.layers = 1
		};

		framebuffers[i] = device->createFramebuffer(ci);
	}

	std::cout << "framebuffers created" << std::endl;

	return true;
}

bool renderer::createCommandPool() {
	CommandPoolCreateInfo ci{ .queueFamilyIndex = 0,
	};

	commandPool = device->createCommandPool(ci);

	std::cout << "command pool created" << std::endl;

	return true;
}

bool renderer::createCommandBuffers() {
	commandBuffers.resize(framebuffers.size());

	CommandBufferAllocateInfo ci{ .commandPool = commandPool,
	.level = CommandBufferLevel::ePrimary,
	.commandBufferCount = static_cast<uint32_t>(commandBuffers.size()) };

	commandBuffers = device->allocateCommandBuffers(ci);

	for (size_t i = 0; i < commandBuffers.size(); i++) {
		ClearValue clearColor = { std::array<float,4>{0.0f, 0.0f, 0.0f, 1.0f} };

		CommandBufferBeginInfo info{};

		commandBuffers[i].begin(info);

		RenderPassBeginInfo rpInfo{
			.renderPass = rp,
			.framebuffer = framebuffers[i],
			.renderArea = {.offset = {0, 0}, .extent = extent},
			.clearValueCount = 1,
			.pClearValues = &clearColor };

		commandBuffers[i].beginRenderPass(rpInfo, SubpassContents::eInline);

		commandBuffers[i].bindPipeline(PipelineBindPoint::eGraphics, pipeline);
		
		Buffer vbs[] = { vb };
		DeviceSize offsets[] = { 0 };
		
		commandBuffers[i].bindVertexBuffers(0, 1, vbs, offsets);
		commandBuffers[i].draw(static_cast<uint32_t>(vertices.size()), 1, 0, 0);

		commandBuffers[i].endRenderPass();
		commandBuffers[i].end();
	}

	std::cout << "command buffers allocated" << std::endl;

	return true;
}

bool renderer::createSemaphores() {

	SemaphoreCreateInfo ci{};

	imgSemaphore = device->createSemaphore(ci);
	renderSemaphore = device->createSemaphore(ci);

	std::cout << "semaphores created" << std::endl;

	return true;
}

bool renderer::createFence() {

	FenceCreateInfo ci{ .flags = FenceCreateFlagBits::eSignaled };
	fence = device->createFence(ci);

	std::cout << "fence created" << std::endl;

	return true;
}

void renderer::render() {

	device->waitForFences(fence, VK_TRUE, UINT64_MAX);

	device->resetFences(fence);

	uint32_t imgIndex;

	imgIndex = device->acquireNextImageKHR(swapchain, UINT64_MAX, imgSemaphore, nullptr).value;

	Semaphore waitSemaphores[] = { imgSemaphore };
	Semaphore signalSemaphores[] = { renderSemaphore };

	PipelineStageFlags waitDstStages[] = { PipelineStageFlagBits::eColorAttachmentOutput };

	SwapchainKHR swapchains[] = { swapchain };

	SubmitInfo info{ .waitSemaphoreCount = 1,
	.pWaitSemaphores = waitSemaphores,
	.pWaitDstStageMask = waitDstStages,
	.commandBufferCount = 1,
	.pCommandBuffers = &commandBuffers[imgIndex],
	.signalSemaphoreCount = 1,
	.pSignalSemaphores = signalSemaphores };

	gfxQueue.submit(info);

	PresentInfoKHR presentInfo{ .waitSemaphoreCount = 1,
	.pWaitSemaphores = signalSemaphores,
	.swapchainCount = 1,
	.pSwapchains = swapchains,
	.pImageIndices = &imgIndex };

	presentQueue.presentKHR(presentInfo);



}





	
