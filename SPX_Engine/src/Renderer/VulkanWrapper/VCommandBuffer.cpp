#include "VCommandBuffer.h"

VkCommandBuffer VCommandBuffer::beginSingleTimeCommands(VkCommandPool commandPool, VkDevice device) {
	// Memory transfer operations are excuted using command buffers, just like drawing commands.
	// MIGHT WANT TO CREATE A COMMAND POOL FOR TEMPORARY BUFFERS, because the implementation may be able
	// to apply memory allocation optimizations. I should use VK_COMMAND_POOL_CREATE_TRANSIENT_BIT flag during command pool
	// generation in that case
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void VCommandBuffer::endSingleTimeCommands(VkCommandPool commandPool, VkCommandBuffer& commandBuffer, VkQueue graphicsFamily, VkDevice device) {
	// Execute the command buffer to complete the transfer.
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(graphicsFamily, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(graphicsFamily);

	vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}
