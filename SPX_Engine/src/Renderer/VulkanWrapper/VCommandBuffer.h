#pragma once

#include "../../pch.h"

class VDevice;

class VCommandBuffer {
public:
	VkCommandBuffer mCommandBuffer;

	static VkCommandBuffer beginSingleTimeCommands(VkCommandPool commandPool, VkDevice device);
	static void endSingleTimeCommands(VkCommandPool commandPool, VkCommandBuffer& commandBuffer, VkQueue graphicsFamily, VkDevice device);

private:
	VDevice& mDevice;
};