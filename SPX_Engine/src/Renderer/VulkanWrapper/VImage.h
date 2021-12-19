#pragma once

#include "../../pch.h"
#include "../../ThirdParty/vk_mem_alloc.h"

class VDevice;


// Image and ImageView are created within the constructor of the VImage.
class VImage
{
public:
	VImage(const VDevice& device,
		VkFormat format,
		VkImageUsageFlags imageUsage,
		VkImageAspectFlags aspectFlags,
		VkSampleCountFlagBits sampleCount,
		const std::string& name,
		VkExtent2D imageExtent);

	~VImage();


	static void transitionImageLayout(
		VkImage image,
		VkFormat format,
		VkImageLayout oldLayout,
		VkImageLayout newLayout,
		VkCommandPool commandPool,
		VkQueue graphicsFamily,
		VkDevice device);

	static void copyBufferToImage(
		VkBuffer buffer,
		VkImage image,
		uint32_t width,
		uint32_t height,
		VkCommandPool commandPool,
		VkQueue graphicsFamily,
		VkDevice device);

	static bool hasStencilComponent(VkFormat format)
	{
		return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
	}

	// TODO: Make private and set getter functions
	const VDevice& mDevice;
	VmaAllocation mAllocation{ VK_NULL_HANDLE };
	VmaAllocationInfo mAllocationInfo{ VK_NULL_HANDLE };
	VkImage mImage{ VK_NULL_HANDLE };
	VkFormat mFormat{ VK_FORMAT_UNDEFINED };
	VkImageView mImageView{ VK_NULL_HANDLE };
	std::string mName; // ?? Idk about keeping this.



private:
};