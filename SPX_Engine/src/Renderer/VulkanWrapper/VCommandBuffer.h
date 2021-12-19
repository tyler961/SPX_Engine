#pragma once

#include "../../pch.h"

// Does this really need to be a class??

class VDevice;

class VCommandBuffer
{
public:
	VkCommandBuffer mCommandBuffer;

	static VkCommandBuffer beginSingleTimeCommands(VkCommandPool commandPool, VkDevice device);
	static void endSingleTimeCommands(VkCommandPool commandPool, VkCommandBuffer& commandBuffer, VkQueue graphicsFamily, VkDevice device);

private:
	VDevice& mDevice;
};