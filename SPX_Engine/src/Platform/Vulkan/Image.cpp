#include "Image.h"

Image::Image()
{
}

void Image::createImage(
	SPX_INT width, SPX_INT height, 
	VkFormat format, 
	VkImageTiling tiling,
	VkImageUsageFlags usage,
	VkMemoryPropertyFlags props, 
	VkImage& image, 
	VkDeviceMemory& imageMemory,
	VkDevice device,
	VkPhysicalDevice physicalDevice)
{
	// Images are for 2D, better to use image than to put it in a vertex buffer. Reference 201
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1; // This will change if I add miplevels.
	imageInfo.arrayLayers = 1;
	// Use the same format as the texels as the pixels in the buffer or it will fail.
	imageInfo.format = format;
	// Tiling can have two options
	// VK_IMAGE_TILING_OPTIMAL: Texels are laid out in an implementation defined order for optimal access
	// VK_IMAGE_TILING_LINEAR: Texels are laid out in a row-major order like our pixels array
	// Cannot be changed at a later time
	imageInfo.tiling = tiling;
	// Two options:
	// VK_IMAGE_LAYOUT_UNDEFINED: Not usable by the GPU and the very first transition will discard the texels.
	// VK_IMAGE_LAYOUT_PREINTIALIZED: Not useable by the GPU, but the first transition will preserve the texels.
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS)
		CORE_ERROR("Failed to create image.");
	else
		CORE_INFO("Image created successfully.");

	// Allocating memory for an image is the same as for a buffer
	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(device, image, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, props, physicalDevice);

	if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
		CORE_ERROR("Failed to allocate Image Memory.");

	vkBindImageMemory(device, image, imageMemory, 0);
}

SPX_INT Image::findMemoryType(SPX_INT typeFilter, VkMemoryPropertyFlags props, VkPhysicalDevice physicalDevice)
{
	// First need to query info about the available types of memory
	// This struct has two arrays, memoryTypes and memoryHeaps. Heads are distinct memory resources like dedicated VRAM
	// ans swap space in RAM when VRAM runs out. 
	// For now I'll only concern myself with the type of memeory and not the heap it comes from. THIS CAN AFFECT PREFORMANCE
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

	// PG 161
	// Find the first memory type that is suitabe for the buffer itself.
	// Type filter will be used to specify the bit field of memory types that are suitable.
	// Not use interested in a memory type that is suitable for vertex buffer. Also need to be able to write 
	// the vertex data to that memory.
	// Loop to also check for the support of this property.
	for (SPX_INT i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & props) == props)
			return i;
	}

	CORE_ERROR("Failed to find suitable memory type.");
}

void Image::transitionImageLayout(
	VkImage image, 
	VkFormat format, 
	VkImageLayout oldLayout,
	VkImageLayout newLayout,
	VkCommandPool commandPool,
	VkQueue graphicsFamily,
	VkDevice device)
{
	VkCommandBuffer commandBuffer = CommandBuffer::beginSingleTimeCommands(commandPool, device);

	// Used to preform layout transitions. page 206
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;

	// Pg 237, checking if it's supposed to be depth or not
	if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		if (hasStencilComponent(format))
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
	}
	else
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;


	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	else {
		throw std::invalid_argument("unsupported layout transition!");
	}

	// Pg 207-208
	vkCmdPipelineBarrier(
		commandBuffer,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	CommandBuffer::endSingleTimeCommands(commandPool, commandBuffer, graphicsFamily, device);
}

void Image::copyBufferToImage(
	VkBuffer buffer,
	VkImage image, 
	SPX_INT width, 
	SPX_INT height, 
	VkCommandPool commandPool,
	VkQueue graphicsFamily,
	VkDevice device)
{
	VkCommandBuffer commandBuffer = CommandBuffer::beginSingleTimeCommands(commandPool, device);

	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = {
		width,
		height,
		1
	};

	vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	CommandBuffer::endSingleTimeCommands(commandPool, commandBuffer, graphicsFamily, device);
}
