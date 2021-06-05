#pragma once

#include "../../pch.h"
#include "CommandBuffer.h"

struct Image
{
	Image();
	static void createImage(
		SPX_INT width, SPX_INT height, 
		VkFormat format, 
		VkImageTiling tiling, 
		VkImageUsageFlags usage,
		VkMemoryPropertyFlags props, 
		VkImage& image, 
		VkDeviceMemory& imageMemory,
		VkDevice device,
		VkPhysicalDevice physicalDevice); 

	static SPX_INT findMemoryType(
		SPX_INT typeFilter, 
		VkMemoryPropertyFlags props, 
		VkPhysicalDevice physicalDevice);

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
		SPX_INT width,
		SPX_INT height,
		VkCommandPool commandPool,
		VkQueue graphicsFamily,
		VkDevice device);

	static bool hasStencilComponent(VkFormat format)
	{ return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT; }
};