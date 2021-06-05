#pragma once

#include "../../pch.h"
#include "Image.h"
#include "CommandBuffer.h"

struct FrameBuffer
{
	FrameBuffer();

	static void createFrameBuffers(VkExtent2D extent,
		std::vector<VkFramebuffer>& swapChainFrameBuffers,
		std::vector<VkImageView> swapChainImageViews,
		VkImageView depthImageView,
		VkRenderPass mRenderPass,
		VkDevice device);

	static void createBuffer(VkDeviceSize size, 
		VkBufferUsageFlags usage, 
		VkMemoryPropertyFlags props,
		VkBuffer& buffer, 
		VkDeviceMemory& bufferMemory,
		VkPhysicalDevice physicalDevice,
		VkDevice device);

	static void copyBuffer(
		VkBuffer& srcBuffer,
		VkBuffer& dstBuffer,
		VkDeviceSize size,
		VkCommandPool commandPool,
		VkQueue graphicsFamily,
		VkDevice device);
};