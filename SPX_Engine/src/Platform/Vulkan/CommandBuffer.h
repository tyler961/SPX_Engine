#pragma once

#include "../../pch.h"

struct CommandBuffer
{
	CommandBuffer();

	void createCommandBuffers(
		std::vector<VkFramebuffer> swapChainFramebuffers,
		VkCommandPool commandPool,
		VkRenderPass renderPass,
		VkExtent2D swapChainExtent,
		VkPipeline graphicsPipeline,
		VkPipelineLayout pipelineLayout,
		VkBuffer vertexBuffer,
		VkBuffer indexBuffer,
		std::vector<VkDescriptorSet> descriptorSets,
		std::vector<SPX_INT> indices,
		VkDevice device);

	static VkCommandBuffer beginSingleTimeCommands(VkCommandPool commandPool, VkDevice device);

	static void endSingleTimeCommands(
		VkCommandPool commandPool, 
		VkCommandBuffer& commandBuffer, 
		VkQueue graphicsFamily, 
		VkDevice device);


	std::vector<VkCommandBuffer> mCommandBuffers;
};