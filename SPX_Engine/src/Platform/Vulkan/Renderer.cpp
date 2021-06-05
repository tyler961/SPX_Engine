#include "Renderer.h"

Renderer::Renderer(Window* window)
	: mWindow(window), mDevice(nullptr), mSwapChain(nullptr), mPipeline(nullptr)
{
	createInstance();
	setupDebug();
	createSurface();
	mDevice = new Device(mSurface);
	mDevice->pickPhysicalDevice(mInstance);
	mDevice->createLogicalDevice(mValLayers);
	mSwapChain = new SwapChain(mDevice->mPhysicalDevice, mDevice->mLogicalDevice, mSurface);
	mSwapChain->createSwapChain(mWindow);
	mSwapChain->createSwapChaimImageViews(mDevice->mLogicalDevice);
	createRenderPass();
	createDescriptorSetLayout();
	mPipeline = new GraphicsPipeline("src/ShaderFiles/vert.spv", "src/ShaderFiles/frag.spv", mDevice->mLogicalDevice);
	mPipeline->createGraphicsPipeline(mDevice->mLogicalDevice, mSwapChain->mSwapChainExtent, mDescriptorSetLayout, mRenderPass);
	createCommandPool();
}

void Renderer::createInstance()
{
	if (mValLayers.mEnableValidationLayers && !mValLayers.checkValidationLayerSupport())
		CORE_ERROR("Validation layers requested, but not available.");

	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "SPX Engine"; // TODO Change to apply to the application
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0); // Change when farther along
	appInfo.pEngineName = "SPX Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0); // Change when farther along
	appInfo.apiVersion = VK_API_VERSION_1_0;


	// Get extension info for the glfwWindow
	SPX_INT glfwExtensionsCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionsCount);
	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionsCount);

	// IF I DONT DO THIS DEBUG DIES AND WONT LOAD
	if (mValLayers.mEnableValidationLayers)
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);


	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledExtensionCount = static_cast<SPX_INT>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	// Doing this here lets validation layers to be used during VkCreateInstance and VkDestroyInstance
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
	if (mValLayers.mEnableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<SPX_INT>(mValLayers.mValidationLayers.size());
		createInfo.ppEnabledLayerNames = mValLayers.mValidationLayers.data();
		
		mValLayers.populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
	else
	{
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
	}

	if (vkCreateInstance(&createInfo, nullptr, &mInstance) != VK_SUCCESS)
		CORE_ERROR("Failed to create Vulkan Instance.");
	else
		CORE_INFO("Vulkan Instance created successfully.");
}

void Renderer::setupDebug()
{
	mValLayers.setupDebugMessenger(mInstance);
}

void Renderer::createSurface()
{
	// Link the window from GLFW to vulkan
	if (glfwCreateWindowSurface(mInstance, mWindow->getContext(), nullptr, &mSurface) != VK_SUCCESS)
		CORE_ERROR("Failed to create Vulkan Surface.");
	else
		CORE_INFO("Vulkan Surface created successfully.");
}

void Renderer::createRenderPass()
{
	// This is to tell Vulkan about the framebuffer attachments that I will be using during rendering. I need
	// to specify how many color and depth buffers there will be, how many samples to use for each of them and how
	// their contents should be handled throughout the rendering operations. All of this info is wrapped in a render
	// pass object, for which I'll create a new createRenderPass function.
	// In this case I'll have just a single color buffer attachment represented by one of the images from the swap chain
	// This has a lot of settings that I will need to dynamically change once I actually begin an engine.
	VkAttachmentDescription colorAttach{};
	colorAttach.format = mSwapChain->mSwapChainImageFormat;
	// With no multisampling yet, setting it to one sample
	colorAttach.samples = VK_SAMPLE_COUNT_1_BIT;
	// Determines what to do with data in the attachment before rendering and after rendering.
	colorAttach.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // Clear the values to a constant at the start
	colorAttach.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // Rendered contents will be stored in memory and can be read later
	// No stencil yet so set to idc
	colorAttach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	// InitalLayout specifies which layout the image will have before render pass
	colorAttach.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	// FinalLayout specifies the layout to automatically transition to when the render pass finishes.
	// I want the image to be ready for presentation using the swapchain after rendering.
	colorAttach.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentDescription depthAttach{};
	depthAttach.format = findDepthFormat();
	depthAttach.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttach.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttach.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttach.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttach.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	// pg 238
	VkAttachmentReference depthAttachRef = {};
	depthAttachRef.attachment = 1;
	depthAttachRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;


	// This array only has one description so index attachment set to 0 for the start of the array.
	VkAttachmentReference colorAttachRef{};
	colorAttachRef.attachment = 0;
	// Layout specifies which layout I would like the attachment to have during a subpass that uses this reference.
	colorAttachRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;


	// Subpass
	// Learn more about subasses.
	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; // Need to specifiy Graphics over Compute.
	// The index of the attachment in this array DIRECTLY REFERENCES the fragment shader with layout(location = 0) out vec4 outColor.
	// These other types of attachements can also be referenced.
	// pInputAttachments: Attachments that are read from a shader
	// pResolveAttachments: Attachments used for multisampling color attachments
	// pDepthStencilAttachment: Attachment for depth and stencil data
	// pPreserveAttachments: Attachments that are not used by this subpass, but for which the data must be preseved. 
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachRef;
	// Here is where depth attachment is hooked:
	subpass.pDepthStencilAttachment = &depthAttachRef;

	VkSubpassDependency depend{};
	depend.srcSubpass = VK_SUBPASS_EXTERNAL;
	depend.dstSubpass = 0;
	depend.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	depend.srcAccessMask = 0;
	depend.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	depend.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	// Now that the attachment and basic subpass referencing have been described, I can create the render pass.
	// The render pass object can be created by filling out the below structure with what I've created before.
	std::array<VkAttachmentDescription, 2> attachments = { colorAttach, depthAttach }; // Here I add depthAttach once I've done that.
	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<SPX_INT>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &depend;

	if (vkCreateRenderPass(mDevice->mLogicalDevice, &renderPassInfo, nullptr, &mRenderPass) != VK_SUCCESS)
		CORE_ERROR("Failed to create a render pass.");
	else
		CORE_INFO("Render pass created successfully.");

}

void Renderer::createDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding uboLayoutBinding{};
	// First two specify the binding used in the shader and the type of decriptor, which is a uniform buffer object.
	// It is possible for the shader variable to represent an array of uniform buffer objects
	// Descriptor count specifies the number of values in an array.
	// This could be used to specify a transformation for each of the bones in a skeleton for skeletal animation,
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	// Only relevant for an image sampling related descriptors.
	uboLayoutBinding.pImmutableSamplers = nullptr;
	// Specify in what shader stages the descriptor is going to be referenced.
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	// Page 221
	VkDescriptorSetLayoutBinding samplerLayoutBinding{};
	samplerLayoutBinding.binding = 1;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.pImmutableSamplers = nullptr;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };
	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(mDevice->mLogicalDevice, &layoutInfo, nullptr, &mDescriptorSetLayout) != VK_SUCCESS)
		CORE_ERROR("Failed to create descriptor layout.");
	else
		CORE_INFO("Descriptor Layout created successfully.");	
}

void Renderer::createCommandPool()
{
	// Command buffers are executed by submitting them on one of the device queues, like the graphics and presentation que
	// I retrieved abovw. Each command pool can only allocate command buffers that are submitted on a single type of queue.
	// I'm going to record commands for drawing, which is why I've chosen the graphics queue family.
	// Two possible flags for command pools:
	// VK_COMMAND_POOL_CREATE_TRANSIENT_BIT: Hint that command buffers are recorded with new commands very often
	// VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT: Allow command buffers to be rerecorded individually, without this flag they all have to be reset together
	// I'll only record the command buffers at the beginning of the program then execute them many times in the main loop.
	// So not going to use either of these flags
	QueueFamilyIndices queueFamilyIndices = Device::findQueueFamilies(mDevice->mPhysicalDevice, mSurface);

	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
	poolInfo.flags = 0;

	if (vkCreateCommandPool(mDevice->mLogicalDevice, &poolInfo, nullptr, &mCommandPool) != VK_SUCCESS)
		CORE_ERROR("Failed to create Command Pool.");
	else
		CORE_INFO("Command Pool creation successful.");
}

VkFormat Renderer::findDepthFormat()
{
	return findSupportedFormat(
		{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
		VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);
}

VkFormat Renderer::findSupportedFormat(
	const std::vector<VkFormat>& candidates,
	VkImageTiling tiling, 
	VkFormatFeatureFlags features)
{
	for (VkFormat format : candidates)
	{
		VkFormatProperties prop;
		vkGetPhysicalDeviceFormatProperties(mDevice->mPhysicalDevice, format, &prop);

		if (tiling == VK_IMAGE_TILING_LINEAR && (prop.linearTilingFeatures & features) == features)
			return format;
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (prop.optimalTilingFeatures & features) == features)
			return format;
	}

	CORE_ERROR("Failed to find a supported format.");
}

