#include "Renderer.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "../../tiny_obj_loader/tiny_obj_loader.h"

Renderer::Renderer(Window* window)
	: mWindow(window), mDevice(nullptr), mSwapChain(nullptr), mPipeline(nullptr), mCommandBuffer(nullptr), mTexture(nullptr)
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
	createDepthResources();
	mSwapChain->createSwapChainFrameBuffers(mDevice->mLogicalDevice, mDepthImageView, mRenderPass);
	mTexture = new Texture("Media/Textures/chalet.jpg");
	mTexture->createTextureImage(mDevice->mLogicalDevice, mDevice->mPhysicalDevice, mCommandPool, mDevice->mGraphicsQueue);
	mTexture->createTextureImageView(mDevice->mLogicalDevice);
	mTexture->createTextureSampler(mDevice->mPhysicalDevice, mDevice->mLogicalDevice);
	loadModel("Media/Obj/chalet.obj");
	createVertexBuffer();
	createIndexBuffer();
	createUniformBuffers();
	createDescriptorPool();
	createDescriptorSets();
	mCommandBuffer = new CommandBuffer();
	mCommandBuffer->createCommandBuffers(mSwapChain->mSwapChainFrameBuffers, mCommandPool, mRenderPass, mSwapChain->mSwapChainExtent,
		mPipeline->mGraphicsPipeline, mPipeline->mPipelineLayout, mVertexBuffer, mIndexBuffer, mDescriptorSets, indices, mDevice->mLogicalDevice);
	createSyncObjects();
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

void Renderer::createDepthResources()
{
	VkFormat depthFormat = findDepthFormat();

	Image::createImage(mSwapChain->mSwapChainExtent.width, mSwapChain->mSwapChainExtent.height, depthFormat,
		VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		mDepthImage, mDepthImageMemory, mDevice->mLogicalDevice, mDevice->mPhysicalDevice);

	mDepthImageView = ImageView::createImageView(mDepthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, mDevice->mLogicalDevice);
}

void Renderer::loadModel(std::string modelPath)
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelPath.c_str())) {
		throw std::runtime_error(warn + err);
	}

	std::unordered_map<Vertex, SPX_INT> uniqueVertices{};

	for (const auto& shape : shapes)
	{
		for (const auto& index : shape.mesh.indices)
		{
			Vertex vertex{};

			vertex.pos =
			{
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};

			vertex.texCoord =
			{
				attrib.texcoords[2 * index.texcoord_index + 0],
				1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
			};

			vertex.color = { 1.0f, 1.0f, 1.0f };

			if (uniqueVertices.count(vertex) == 0)
			{
				uniqueVertices[vertex] = static_cast<SPX_INT>(vertices.size());
				vertices.push_back(vertex);
			}

			indices.push_back(uniqueVertices[vertex]);
		}
	}

	CORE_INFO("Model loaded successfully.");
}

void Renderer::createVertexBuffer()
{
	// Going to create a visible buffer as a temporary buffer and use
	// a device local one as actual vertex buffer. CHANGED TO THIS PG 168
	// Using the stagingBuffer and stagingBufferMemory for mapping and copying the vertex data.
	VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	FrameBuffer::createBuffer(
		bufferSize, 
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
		stagingBuffer, 
		stagingBufferMemory,
		mDevice->mPhysicalDevice,
		mDevice->mLogicalDevice);

	// pg 163
	// This allows me to access a region of the specified memory resource defined by an offset and size.
	// Possible to specify the special value VK_WHOLE_SIZE to mapp all of the memory.
	// Second to last param is used to specify flags, but there aren't any available yet.
	// Last param specifies the output for the pointer to the mapped memory
	void* data; // Find out wtf this is
	vkMapMemory(mDevice->mLogicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
	// use memcpy the vertex data to the mapped memory and unmap it again.
	// Possible problems: The driver may not immediately copy the data into the buffer memory, for ex bc of caching.
	// Also possible that it writes to the buffer and isnt visible in the mapped memroy yet. Two ways to deal with that problem
	// Use a memory heap that is host coherent, indicated with VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	// Call vkFlushMappedMemoryRanges to after writing to the mapped memory, and call vkInvailidateMappedMemoryRanges before
	// reading from the mapped memory.
	// Went with the first aproach for now. Look into the other on. PAGE 163
	memcpy(data, vertices.data(), (size_t)bufferSize);
	vkUnmapMemory(mDevice->mLogicalDevice, stagingBufferMemory);

	FrameBuffer::createBuffer(
		bufferSize, 
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
		mVertexBuffer, 
		mVertexBufferMemory,
		mDevice->mPhysicalDevice,
		mDevice->mLogicalDevice);

	FrameBuffer::copyBuffer(
		stagingBuffer, 
		mVertexBuffer, 
		bufferSize, 
		mCommandPool,
		mDevice->mGraphicsQueue,
		mDevice->mLogicalDevice);

	vkDestroyBuffer(mDevice->mLogicalDevice, stagingBuffer, nullptr);
	vkFreeMemory(mDevice->mLogicalDevice, stagingBufferMemory, nullptr);

	CORE_INFO("Vertex Buffer created successfully.");
}


// Going to be very similar to vertex buffer
// Two differences. The buffersize is now equal to the number of indices times the size of the index type.
// The usage of the indexBuffer should be VK_BUFFER_USAGE_INDEX_BUFFER_BIT of course.
// Otherwise it's exactly the same
// Create a staging buffer to copy the contents of indices to and then copy it to the final device local index buffer.
void Renderer::createIndexBuffer()
{
	VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	FrameBuffer::createBuffer(
		bufferSize, 
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
		stagingBuffer, 
		stagingBufferMemory,
		mDevice->mPhysicalDevice,
		mDevice->mLogicalDevice);

	void* data;
	vkMapMemory(mDevice->mLogicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, indices.data(), (size_t)bufferSize);
	vkUnmapMemory(mDevice->mLogicalDevice, stagingBufferMemory);

	FrameBuffer::createBuffer(
		bufferSize, 
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
		mIndexBuffer, 
		mIndexBufferMemory,
		mDevice->mPhysicalDevice,
		mDevice->mLogicalDevice);

	FrameBuffer::copyBuffer(
		stagingBuffer, 
		mIndexBuffer, 
		bufferSize,
		mCommandPool,
		mDevice->mGraphicsQueue,
		mDevice->mLogicalDevice);

	vkDestroyBuffer(mDevice->mLogicalDevice, stagingBuffer, nullptr);
	vkFreeMemory(mDevice->mLogicalDevice, stagingBufferMemory, nullptr);

	CORE_INFO("Index Buffer created successfully.");
}

void Renderer::createUniformBuffers()
{
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);

	mUniformBuffers.resize(mSwapChain->mSwapChainImages.size());
	mUniformBuffersMemory.resize(mSwapChain->mSwapChainImages.size());

	for (size_t i = 0; i < mSwapChain->mSwapChainImages.size(); i++) {
		FrameBuffer::createBuffer(
			bufferSize, 
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			mUniformBuffers[i], 
			mUniformBuffersMemory[i],
			mDevice->mPhysicalDevice,
			mDevice->mLogicalDevice);
	}

	CORE_INFO("Uniform Buffers created successfully.");
}

// Page 187
// Updated page 222
void Renderer::createDescriptorPool()
{
	// Describe which descriptor types my descriptor sets are going to contain and how many of them
	std::array<VkDescriptorPoolSize, 2> poolSizes{};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	// Allocate on of these descriptors for every frame
	poolSizes[0].descriptorCount = static_cast<uint32_t>(mSwapChain->mSwapChainImages.size());
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = static_cast<uint32_t>(mSwapChain->mSwapChainImages.size());

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = static_cast<uint32_t>(mSwapChain->mSwapChainImages.size());

	if (vkCreateDescriptorPool(mDevice->mLogicalDevice, &poolInfo, nullptr, &mDescriptorPool) != VK_SUCCESS)
		CORE_ERROR("Failed to create Descriptor Pool.");
}

void Renderer::createDescriptorSets()
{
	// Create one descriptor set for each swap chain image all with the same layout.
	std::vector<VkDescriptorSetLayout> layouts(mSwapChain->mSwapChainImages.size(), mDescriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = mDescriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(mSwapChain->mSwapChainImages.size());
	allocInfo.pSetLayouts = layouts.data();

	mDescriptorSets.resize(mSwapChain->mSwapChainImages.size());
	if (vkAllocateDescriptorSets(mDevice->mLogicalDevice, &allocInfo, mDescriptorSets.data()) != VK_SUCCESS)
		CORE_ERROR("Failed to allocate descriptor sets.");

	for (size_t i = 0; i < mSwapChain->mSwapChainImages.size(); i++)
	{
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = mUniformBuffers[i];
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformBufferObject);

		// Updated this stuff on page 222-223
		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = mTexture->mTextureImageView;
		imageInfo.sampler = mTexture->mTextureSampler;

		std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = mDescriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfo;

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = mDescriptorSets[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(mDevice->mLogicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}

void Renderer::createSyncObjects()
{
	// Resize to how many frames I want to be worked on at the end of drawFrame()
	mImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	mRenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	mInFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
	mImagesInFlight.resize(mSwapChain->mSwapChainImages.size(), VK_NULL_HANDLE);
	std::cout << "mImagesInFlight: " << mImagesInFlight.size() << std::endl;

	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	// Create fence info struct
	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	// NEEDED since fences are created in an unsignaled state. It will wait forever if this is not set freezing the program
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	// Loop to build all the semaphores needed so each frame has it's own
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		if (vkCreateSemaphore(mDevice->mLogicalDevice, &semaphoreInfo, nullptr, &mImageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(mDevice->mLogicalDevice, &semaphoreInfo, nullptr, &mRenderFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(mDevice->mLogicalDevice, &fenceInfo, nullptr, &mInFlightFences[i]) != VK_SUCCESS)
			CORE_ERROR("Failed to create synchronization objects for a frame.");
	}
}

void Renderer::updateUniformBuffer(SPX_INT currentImage)
{
	// Calculate time in seconds
	static auto startTime = std::chrono::high_resolution_clock::now();
	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	UniformBufferObject ubo = {};
	// Rotate 90 degress per second
	// existing transform, rotation angle and rotation axis as prams
	ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	// Look at the geometry from above at 45 degree angle.
	// eyepos, center pos, up axis are the params
	ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	// Perspective projection with 45 degree vertial field of view.
	// Next param is aspect ratio, near and far view planes.
	// Important to use current swapchain extent incase the window is resized.
	ubo.proj = glm::perspective(glm::radians(45.0f), mSwapChain->mSwapChainExtent.width / (float)mSwapChain->mSwapChainExtent.height, 0.1f, 10.0f);
	// GLM has the Y coordinate flipped, so I have to flip it by timesing by -1 or it will be rendered upsidedown
	ubo.proj[1][1] *= -1;

	// All transforms are defined now, so I can copy the data in the uniform buffer obj to the current uniform buffer.
	// This happens the same as vertex buffer, but without the staging buffer becuase it gets called so often, it creates too much overhead
	void* data;
	vkMapMemory(mDevice->mLogicalDevice, mUniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data);
	memcpy(data, &ubo, sizeof(ubo));
	vkUnmapMemory(mDevice->mLogicalDevice, mUniformBuffersMemory[currentImage]);

	// Doing uniform buffers this way is not the most efficent way to pass frequently changing values to teh shader.
	// A more efficent way to pass a small buffer of data to shaders are PUSH CONSTANTS.
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