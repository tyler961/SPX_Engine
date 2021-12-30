#include "RenderObject.h"
#include "Mesh.h"
#include "Texture.h"
#include "VulkanWrapper/VDevice.h"
#include "VulkanWrapper/VImage.h"
#include "VulkanWrapper/VCommandPool.h"

RenderObject::RenderObject(std::string meshLoc, std::string textureLoc)
	:mMeshFileLocation(meshLoc), mTextureFileLocation(textureLoc) {
	mTransformMatrix = glm::mat4(1.0f);
}

RenderObject::~RenderObject() {}

void RenderObject::drawObject(VkCommandBuffer cmd, VkPipelineLayout pipelineLayout, uint32_t currentImage) {
	// Here I would bind the pipeline that each object has
	// vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
	// Now to bind vertex buffers
	// Later I can combine all the vertex buffers and use offsets to change which one I'm referncing with the bind.
	// This will REDUCE the overhead.
	VkBuffer vertexBuffers[] = { mMesh->mVertexBuffer.mBuffer };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);
	// Bind the Index Buffers
	vkCmdBindIndexBuffer(cmd, mMesh->mIndexBuffer.mBuffer, 0, VK_INDEX_TYPE_UINT32);
	// Here I would bind the RenderObjects specific descriptor set
	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &mDescriptorSets[currentImage], 0, nullptr);
	// Now to draw using the indices and vertex buffers.
	vkCmdDrawIndexed(cmd, static_cast<uint32_t>(mMesh->mIndices.size()), 1, 0, 0, 0);
}

void RenderObject::updateUniformBuffers(VkExtent2D extent, uint32_t currentImage, glm::mat4 cameraViewMatrix) {
	// Need to add position variables to the render object so it can be moved :D
	UniformBufferObject ubo{};
	// existing transform, rotation angle and rotation axis as prams
	ubo.model = mTransformMatrix;
	ubo.view = cameraViewMatrix;
	// Perspective projection with 45 degree vertial field of view.
	// Next param is aspect ratio, near and far view planes.
	// Important to use current swapchain extent incase the window is resized.
	ubo.proj = glm::perspective(glm::radians(45.0f), extent.width / (float)extent.height, 0.1f, 100.0f);
	ubo.proj[1][1] *= -1;

	// All transforms are defined now, so I can copy the data in the uniform buffer obj to the current uniform buffer.
	// This happens the same as vertex buffer, but without the staging buffer becuase it gets called so often, it creates too much overhead
	void* data;
	vmaMapMemory(mDevice->mAllocator, mMesh->mUniformBuffers[currentImage].mAlloc, &data);
	memcpy(data, &ubo, sizeof(ubo));
	vmaUnmapMemory(mDevice->mAllocator, mMesh->mUniformBuffers[currentImage].mAlloc);
}

void RenderObject::init(VCommandPool commandPool, uint32_t swapchainImages) {
	mTexture = new Texture(mTextureFileLocation, *mDevice);
	mMesh = new Mesh(mMeshFileLocation, *mDevice, swapchainImages);
	loadBuffers();
	mTexture->init(commandPool);
	loadDescriptorInfo(swapchainImages);
}

void RenderObject::loadBuffers() {
	mMesh->createBuffers();
}

void RenderObject::loadDescriptorInfo(uint32_t swapchainImages) {
	createDescriptorPool(swapchainImages);
	createDescriptorSets(swapchainImages);
}


// This is used to allocated specific descriptorsets out and the types taht they are and their positions.
void RenderObject::createDescriptorPool(uint32_t swapchainImages) {
	// Create the descriptor pool that the render object will use.
	// Only need a size of two for now with a color texture and uniform buffer.
	// Later when normal, reflection etc are added I'll increase the size.
	std::array<VkDescriptorPoolSize, 2> poolSizes{}; 
	// Uniform Buffer
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = swapchainImages;  // TODO: Double check if this is correct.
	// Combined Image Sampler
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = swapchainImages;

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = swapchainImages;

	if (vkCreateDescriptorPool(mDevice->mLogicalDevice, &poolInfo, nullptr, &mDescriptorPool) != VK_SUCCESS)
		CORE_ERROR("Failed to create Descriptor Pool.");
}

// This describes the types of descriptor sets I'll be using. They neeed to be bound in the same position as the pool has them.
void RenderObject::createDescriptorSetLayout(VDevice& device) {
	// Need to find a better way to initialize the device since this has to be called before init is done.
	mDevice = &device;

	// For now with binding, I'm only using one for Uniform Buffer and the texture coloring.
	// Later I can add more for normal maps etc
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
}

void RenderObject::createDescriptorSets(uint32_t swapchainImages) {
	// Create one descriptor set for each swap chain image all with the same layout.
	std::vector<VkDescriptorSetLayout> layouts(swapchainImages, mDescriptorSetLayout);

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = mDescriptorPool;
	allocInfo.descriptorSetCount = swapchainImages;
	allocInfo.pSetLayouts = layouts.data();

	mDescriptorSets.resize(swapchainImages);

	if (vkAllocateDescriptorSets(mDevice->mLogicalDevice, &allocInfo, mDescriptorSets.data()) != VK_SUCCESS)
		CORE_ERROR("Failed to allocate descriptor sets.");

	// Now I use descriptor writes to actually record the data I want in each descriptor set.
	for (size_t i = 0; i < swapchainImages; i++) {
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = mMesh->mUniformBuffers[i].mBuffer;
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformBufferObject);

		// Updated this stuff on page 222-223
		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = mTexture->mTextureImage->mImageView;
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