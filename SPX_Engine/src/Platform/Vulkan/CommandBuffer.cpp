#include "CommandBuffer.h"

CommandBuffer::CommandBuffer()
{
}

void CommandBuffer::createCommandBuffers(
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
	VkDevice device)
{

	// Resize
	mCommandBuffers.resize(swapChainFramebuffers.size());
	// Command buffers are allocated with the vkAllocateCommandBuffers function, which takes a VkCommandBufferAllocateInfo struct
	// as a pareameter that specifies the command pool and number of buffers to allocate.
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	// Level specifies if the allocated command buffers are primary or secondary command buffers.
	// VK_COMMAND_BUFFER_LEVEL_PRIMARY: Can be submitted to a queue for execution, but cannot be called from other command buffers
	// VK_COMMAND_BUFFER_LEVEL_SECONDARY: Cannot be submitted directly, but can be called from primary command buffers.
	// Won't use secondary command buffers here, but theyre useful for reusing common operations from primary command buffers.
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (SPX_INT)mCommandBuffers.size();

	if (vkAllocateCommandBuffers(device, &allocInfo, mCommandBuffers.data()) != VK_SUCCESS)
		CORE_ERROR("Failed to allocate command buffers.");

	for (size_t i = 0; i < mCommandBuffers.size(); i++)
	{
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		// Flags specify how I'm going to use the command buffer. Following options available:
		// VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT: The command buffer will be rerecorded after executing once
		// VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT: This is a secondary command buffer that will be entirely within a single render pass
		// VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT: The command buffer can be resubmitted while it is also already pending execution.
		// Using the last flag becuase I might already be scheduling the drawing commands for the next frame while the last frame
		// is not finished yet.
		//beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		// This is only relevant for secondary command buffers. It specifies which state to inherit from the calling primary commadn buffers.
		//beginInfo.pInheritanceInfo = nullptr; // Optional

		if (vkBeginCommandBuffer(mCommandBuffers[i], &beginInfo) != VK_SUCCESS)
			CORE_ERROR("Failed to begin recording command buffer.");

		// Drawing starts by beginning the render pass with vkCmdBeginRenderPass.
		// The render pass is configured using some parameters in VkRenderPassBeginInfo struct
		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass;
		// Create a framebuffer for each swapchain image that spcifies it as a color attachment
		renderPassInfo.framebuffer = swapChainFramebuffers[i];
		// Defines the size of the render area. This is where shader loads and stores will take place.
		// Should match the size of the attachments for best preformance
		renderPassInfo.renderArea.offset = { 0,0 };
		renderPassInfo.renderArea.extent = swapChainExtent;

		// Defines the clear color for VK_ATTACHMENT_LOAD_OP_CLEAR
		// Sets it to black alpha 1.0f
		// Order of clear values has to match the order of my attachments in the createFramebuffer
		std::array<VkClearValue, 2> clearValues = {};
		clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };
		renderPassInfo.clearValueCount = static_cast<SPX_INT>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		// Render pass can now begin. All the functions that record the commands can be recgonized by their vkCmd preface.
		// They all return void so there will be no error handling until we've finished recording.
		// Final parameter controls how the drawing commands within the render pas will be provided.
		// Two Values:
		// VK_SUBPASS_CONTENTS_INLINE: The render pass commands will be embedded in the primary commadn buffer itself
		// and no secondary command buffers will be executed.
		// VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS: The render pass commands will be executed from secondary command buffers
		vkCmdBeginRenderPass(mCommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		// Time to bind the graphics pipeline
		// The second parameter specifies if the pipeline object is a graphics or compute pipeline. 
		// We've told Vulkan which operations to execute in the graphics pipeline and which attachment to use in
		// in the fragment shader.
		vkCmdBindPipeline(mCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

		// Bind the vertex buffer during rendering operations.
		VkBuffer vertexBuffers[] = { vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		// Second and thrid params specify the offset and number of bindings I'm going to specify vertex buffers for.
		// The last two specify the array of vertex buffers to bind and the byte offset to start reading vertex data from.
		vkCmdBindVertexBuffers(mCommandBuffers[i], 0, 1, vertexBuffers, offsets);

		// Similar to Vertex Buffers.
		// Difference is that I can only have a single index buffer.
		// Not possible to use different indices for each vertex attribute so I still have to completely duplicate
		// vertex data even if just one attribute varies
		vkCmdBindIndexBuffer(mCommandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT32);

		// Bind the right descriptor set for each swap chain image to the descriptors in the shader
		vkCmdBindDescriptorSets(mCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1,
			&descriptorSets[i], 0, nullptr);

		// Now to tell it to draw the triangle
		// It is simple because of all the information that I've specified before hand.
		// Second Parameter: VertexCount, Even without a vertex buffer, we technically still have 3 vertices to draw. With buffer, Changed form 3 to its current form
		// Third: instanceCount, used for instanced rendering, use 1 if I'm not doing that
		// Fourth: firstVertex, used as an offset into the vertex buffer, defines the lowest value of gl_VertexIndex.
		// Fifth: firstInstance, used as an offset for instanced rendering, defines the lowest value of gl_InstanceIndex.
		// OLD CALL: vkCmdDraw(mCommandBuffers[i], static_cast<uint32_t>(vertices.size()), 1, 0, 0);
		// New call below with index buffer
		// First two specify the number of indices and number of instances. Not using instancing, so just a 1.
		// Number of indices is the number of vertices that will be passed to the vertex buffer.
		// Next param offset into the index buffer, using a value of 1 would cause the graphics card to read from the second index
		// The second to last param specifies an offset to add to the indices in the index buffer.
		// Final specifies an offset for instancing, which I'm not using.
		vkCmdDrawIndexed(mCommandBuffers[i], static_cast<SPX_INT>(indices.size()), 1, 0, 0, 0);


		// Render pass can now be ended:
		vkCmdEndRenderPass(mCommandBuffers[i]);

		// Finished recording the command buffer:
		if (vkEndCommandBuffer(mCommandBuffers[i]) != VK_SUCCESS)
			CORE_ERROR("Failed to record command buffer!");
	}
}


VkCommandBuffer CommandBuffer::beginSingleTimeCommands(VkCommandPool commandPool, VkDevice device)
{
	// Memory transfer operations are excuted using command buffers, just like drawing commands.
	// MIGHT WANT TO CREATE A COMMAND POOL FOR TEMPORARY BUFFERS, becuase the implementation may be able
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

void CommandBuffer::endSingleTimeCommands(
	VkCommandPool commandPool, 
	VkCommandBuffer& commandBuffer,
	VkQueue graphicsFamily, 
	VkDevice device)
{
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
