#include "VImage.h"
#include "VDevice.h"


// TODO: Allow for changing of tiling
// 
VImage::VImage(
	const VDevice& device, 
	VkFormat format, 
	VkImageUsageFlags imageUsage, 
	VkImageAspectFlags aspectFlags,
	VkSampleCountFlagBits sampleCount, 
	const std::string& name, 
	VkExtent2D imageExtent)
	: mDevice(device), mFormat(format), mName(name)
{
	VkImageCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	createInfo.imageType = VK_IMAGE_TYPE_2D;
	createInfo.extent.width = imageExtent.width;
	createInfo.extent.height = imageExtent.height;
	createInfo.extent.depth = 1;
	createInfo.mipLevels = 1;
	createInfo.arrayLayers = 1;
	createInfo.format = format;
	createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	createInfo.usage = imageUsage;
	// This is for MSAA/multisampling.
	createInfo.samples = sampleCount;
	createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VmaAllocationCreateInfo VmaAlloc{};
	VmaAlloc.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	// TODO: Can add here to record the memory allocation to check it
	// SEE inexor Vulkan Renderer under Image for example
	VmaAlloc.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

	if (vmaCreateImage(mDevice.mAllocator, &createInfo, &VmaAlloc, &mImage, &mAllocation, &mAllocationInfo) != VK_SUCCESS)
		CORE_ERROR("ERROR: vmaCreateImage failed for image {}.", mName);



	// Now create ImageView for this image.
	VkImageViewCreateInfo createViewInfo{};
	createViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createViewInfo.image = mImage;
	createViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createViewInfo.format = format;
	createViewInfo.subresourceRange.aspectMask = aspectFlags;
	createViewInfo.subresourceRange.baseMipLevel = 0;
	createViewInfo.subresourceRange.levelCount = 1;
	createViewInfo.subresourceRange.baseArrayLayer = 0;
	createViewInfo.subresourceRange.layerCount = 1;

	if (vkCreateImageView(mDevice.mLogicalDevice, &createViewInfo, nullptr, &mImageView) != VK_SUCCESS)
		CORE_ERROR("Error: vkCreateImageView failed for image view {}.", mName);
}

VImage::~VImage()
{
	//vkDestroyImageView(mDevice.mLogicalDevice, mImageView, nullptr);
	//vmaDestroyImage(mDevice.mAllocator, mImage, mAllocation);
}