#include "FrameBuffer.h"

FrameBuffer::FrameBuffer()
{
}

void FrameBuffer::createFrameBuffers(
	VkExtent2D extent, 
	std::vector<VkFramebuffer>& swapChainFrameBuffers, 
	std::vector<VkImageView> swapChainImageViews,
	VkImageView depthImageView,
	VkRenderPass mRenderPass, 
	VkDevice device)
{
	// Start by resizing the container to hold all of the framebuffers
	swapChainFrameBuffers.resize(swapChainImageViews.size());

	// Then iterate thorugh the image views and create framebuffers from them
	for (size_t i = 0; i < swapChainImageViews.size(); i++)
	{
		std::array<VkImageView, 2> attachments = { swapChainImageViews[i], depthImageView };

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		// Specify which render pass the frame buffer needs to be compatible with, so they roughly use the same number of attachments
		framebufferInfo.renderPass = mRenderPass;
		// Attachmentcount and pAttachments specify the VkImageView objects that should be bound to the respective attachment
		// descriptions in the render pass pAttachment array
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		// Width and height are self-explanatory and layers refers to the number of laying in image arrays.
		// Since the swap chain images are single images, the number of layers is 1
		framebufferInfo.width = extent.width;
		framebufferInfo.height = extent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFrameBuffers[i]) != VK_SUCCESS)
			CORE_ERROR("Failed to create framebuffer.");
		else
			CORE_INFO("Framebuffers created successfully.");
	}
}

void FrameBuffer::createBuffer(
	VkDeviceSize size,
	VkBufferUsageFlags usage,
	VkMemoryPropertyFlags props, 
	VkBuffer& buffer, 
	VkDeviceMemory& bufferMemory, 
	VkPhysicalDevice physicalDevice,
	VkDevice device)
{
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	// Specifies size of the buffer in bytes
	bufferInfo.size = size;
	// Usage indicates for which purposes the data in the buffer is going to be used.
	// Possible to specify multiple purposes using bitwise or.
	bufferInfo.usage = usage;
	// sharing mode is like images in the swapchain, they can be owned by a specific queue family or between multiple at teh same time.
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	// Create the buffer
	if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
		CORE_ERROR("Failed to create Buffer.");

	// Start assigning memory to the buffer. First find out its memory requirements
	// It has 3 fields:
	// size: the required aout of memory in bytes, may differ from bufferInfo.size
	// alignment: the offset in bytes where teh buffer begins in the allocated reigion of memory, depends on bufferInfo.usage and bufferInfo.flags
	// memoryTypeBits: Bit field of the memory types taht are suitable for the buffer.
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

	// with fineMemoryType, I have a way to determine the right memory type, so time to allocate the memory by filling in this struct
	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = Image::findMemoryType(memRequirements.memoryTypeBits, props, physicalDevice);

	// Now memory allocation is as simple as specifying the size and type. Both are derived from the memory requirements
	// of the vertex buffer and the desired property.
	if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
		CORE_ERROR("Failed to allocate vertex buffer memory.");

	// Now associate this memory with the buffer
	// Fourth param is the offset within the region of memory. Since this memory is allocated specifically for this vertex
	// buffer, the offset is simply 0. If it is non-zero, then it is required to be divisible by memRequirements.alignment.
	// Needs to be freed at some at some point. Memory that is bound to a buffer object may be freed once the buffer is no longer
	// used. 
	vkBindBufferMemory(device, buffer, bufferMemory, 0);
}

void FrameBuffer::copyBuffer(
	VkBuffer& srcBuffer, 
	VkBuffer& dstBuffer, 
	VkDeviceSize size,
	VkCommandPool commandPool,
	VkQueue graphicsFamily,
	VkDevice device)
{
	VkCommandBuffer commandBuffer = CommandBuffer::beginSingleTimeCommands(commandPool, device);

	// the VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT used in drawing command buffers isnt necessary here,
	// because I'm only going to use the command buffer once and wait with returning from the function until
	// the cop operation has finished executing.
	VkBufferCopy copyRegion = {};
	copyRegion.srcOffset = 0; // Optional
	copyRegion.dstOffset = 0; // Optional
	copyRegion.size = size;
	// Getting complicated. However, contents of buffers are transferred using the command below. It takes a source and
	// destination buffer as arguments and an array of regions to copy. The regions are defined in the VkBufferCopy structs
	// and consist of a source buffer offset, destination buffer offset and size. It is not possible to specify VK_WHOLE_SIZE
	// here, unlike the vkMapMemory Command.
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);


	CommandBuffer::endSingleTimeCommands(commandPool, commandBuffer, graphicsFamily, device);
}
