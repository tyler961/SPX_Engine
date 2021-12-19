#include "Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../ThirdParty/stb_image.h"
#include "VulkanWrapper/DataStructures.h"
#include "VulkanWrapper/VulkanHelperFunctions.h"
#include "VulkanWrapper/VDevice.h"
#include "VulkanWrapper/VCommandPool.h"
#include "VulkanWrapper/VCommandBuffer.h"
#include "VulkanWrapper/VImage.h"

Texture::Texture(std::string texturePath, VDevice& device)
	:mDevice(device), mFileLocation(texturePath)
{
	
}


void Texture::init(VCommandPool commandPool)
{
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load(mFileLocation.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

	if (!pixels)
		CORE_ERROR("Failed to load texture image.");


	void* pixel_ptr = pixels;
	VkDeviceSize imageSize = texWidth * texHeight * 4;
	VkExtent2D imageExtent;
	imageExtent.width = static_cast<uint32_t>(texWidth);
	imageExtent.height = static_cast<uint32_t>(texHeight);

	VkExtent3D imageExtent3;
	imageExtent3.width = static_cast<uint32_t>(texWidth);
	imageExtent3.height = static_cast<uint32_t>(texHeight);
	imageExtent3.depth = 1;


	VkFormat imageFormat = VK_FORMAT_R8G8B8A8_SRGB;

	AllocatedBuffer stagingBuffer = VHF::VulkanHelperFunctions::createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, mDevice);

	void* data;
	vmaMapMemory(mDevice.mAllocator, stagingBuffer.mAlloc, &data);
	memcpy(data, pixel_ptr, static_cast<size_t>(imageSize));
	vmaUnmapMemory(mDevice.mAllocator, stagingBuffer.mAlloc);

	stbi_image_free(pixels);

	mTextureImage = new VImage(mDevice, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT, VK_SAMPLE_COUNT_1_BIT, "", imageExtent);

	// Start the command buffer single time to transition the image to transfer-reciever
	VkCommandBuffer cmd = VCommandBuffer::beginSingleTimeCommands(commandPool.mCommandPool, mDevice.mLogicalDevice);
	VkImageSubresourceRange range;
	range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	range.baseMipLevel = 0; // No mip level available yet
	range.levelCount = 1;
	range.baseArrayLayer = 0;
	range.layerCount = 1;

	VkImageMemoryBarrier imageBarrier_toTransfer{};
	imageBarrier_toTransfer.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageBarrier_toTransfer.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageBarrier_toTransfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	imageBarrier_toTransfer.image = mTextureImage->mImage;
	imageBarrier_toTransfer.subresourceRange = range;

	imageBarrier_toTransfer.srcAccessMask = 0;
	imageBarrier_toTransfer.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

	// Barrier the image into the transfer-recieve layout
	vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 
		0, 0, nullptr, 0, nullptr, 1, &imageBarrier_toTransfer);

	VkBufferImageCopy copyRegion{};
	copyRegion.bufferOffset = 0;
	copyRegion.bufferRowLength = 0;
	copyRegion.bufferImageHeight = 0;

	copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	copyRegion.imageSubresource.mipLevel = 0;
	copyRegion.imageSubresource.baseArrayLayer = 0;
	copyRegion.imageSubresource.layerCount = 1;
	copyRegion.imageExtent = imageExtent3;

	// Copy the buffer into the image
	vkCmdCopyBufferToImage(cmd, stagingBuffer.mBuffer, mTextureImage->mImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

	VkImageMemoryBarrier imageBarrier_toReadable = imageBarrier_toTransfer;
	imageBarrier_toReadable.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	imageBarrier_toReadable.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageBarrier_toReadable.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	imageBarrier_toReadable.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	// Barrier the image into the shader readable layout.
	vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier_toReadable);

	VCommandBuffer::endSingleTimeCommands(commandPool.mCommandPool, cmd, mDevice.mGraphicsQueue, mDevice.mLogicalDevice);

	CORE_INFO("Texture loaded.");

	createTextureSampler();
}

void Texture::createTextureSampler()
{
	VkPhysicalDeviceProperties props{};
	vkGetPhysicalDeviceProperties(mDevice.mPhysicalDevice, &props);

	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	// Specifies how to interpolate texels that are magnified or minimized.
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	// Addressing mode can be specified per axis. Available values:
	// VK_SAMPLER_ADDRESS_MODE_REPEAT: Repeat the texture when going beyond the image dimensions
	// VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT: Like repeat, but inverts the coordinates to mirror the image when going beyond
	// the image dimensions
	// VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE: Take the color edge closest to the coordinate beyond the image dimensions
	// VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE: Like clamp to edge, but instead uses the edge opposite to the closet edge
	// VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BOARDER: Return a solid color when sampling beyond the dimensions of the image.
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	// Specifies if anisotropic filtering should be used. No reason NOT to use this unless preformance is concern.
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = props.limits.maxSamplerAnisotropy;
	// Specifies which color is returned when sampling beyond the image.
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	// Specifies which coordinate system I want to use to address texels in an image.
	// Bottom of page 218 explains other options
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	// If enabled then texels will first be compared to a value, and the result of that comparison is used in filtering operations.
	// Mainly used for percentage-closer filtering on shadow maps.
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

	if (vkCreateSampler(mDevice.mLogicalDevice, &samplerInfo, nullptr, &mTextureSampler) != VK_SUCCESS)
		CORE_ERROR("Failed to create a texture sampler.");
	else
		CORE_INFO("Texture Sampler created successfully.");
}
