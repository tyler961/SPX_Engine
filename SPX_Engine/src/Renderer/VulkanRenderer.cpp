#include "VulkanRenderer.h"
#include "VulkanWrapper/VInstance.h"
#include "VulkanWrapper/VSurface.h"
#include "VulkanWrapper/VSwapChain.h"
#include "VulkanWrapper/VDevice.h"
#include "VulkanWrapper/VRenderPass.h"
#include "VulkanWrapper/VGraphicsPipeline.h"
#include "VulkanWrapper/VCommandPool.h"
#include "VulkanWrapper/VFrameBuffer.h"
#include "RenderObject.h"
#include "../SPX/Window.h"
#include "Mesh.h"


VulkanRenderer::VulkanRenderer(Window* window)
	:mWindow(window) {}

VulkanRenderer::~VulkanRenderer() {}

void VulkanRenderer::init(std::string appName, std::string engineName, bool enableValLayers) {
	// When a new model is loaded during runtime, I have to remake some stuff. Especially once I've changed descriptors and pipeline.
	mInstance = new VInstance(appName, engineName, enableValLayers);
	mSurface = new VSurface(mInstance->get(), mWindow);
	mDevice = new VDevice(mSurface->getSurface(), *mInstance);
	mSwapChain = new VSwapChain(*mDevice, mWindow);
	mRenderPass = new VRenderPass(*mDevice, "", *mSwapChain);
	mGraphicsPipeline = new VGraphicsPipeline("src/ShaderFiles/vert.spv", "src/ShaderFiles/frag.spv", *mDevice);
	// This has to be called so the descriptor sets are created.
	for (size_t i = 0; i < mRenderObjects.size(); i++)
		mRenderObjects.at(i).createDescriptorSetLayout(*mDevice);

	mGraphicsPipeline->createGraphicsPipeline(mSwapChain->mSwapChainExtent, mRenderObjects.at(0).mDescriptorSetLayout, mRenderPass->mRenderPass);
	mCommandPool = new VCommandPool(*mDevice, *mSurface);
	mSwapChain->createSwapChainFrameBuffers(*mRenderPass);

	// Load the textures and render objects.
	// Adds the load descriptor info to this function.
	loadRenderObjects();
	// Make sure I have a command buffer for each frame. This will allow me to work on one while the other is being processed by the GPU.
	createCommandBuffers();
	
	// This code is to set their intial model or local position.
	glm::vec3 rotation1 = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::mat4 modelMatrix1 = glm::mat4(1.0f);
	glm::mat4 currentTransform1 = glm::rotate(modelMatrix1, glm::radians(90.0f), rotation1);
	rotation1 = glm::vec3(0.0f, 1.0f, 0.0f);
	currentTransform1 = glm::rotate(currentTransform1, glm::radians(90.0f), rotation1);
	glm::vec3 move1 = glm::vec3(0.0f, -1.5f, 0.0f);
	currentTransform1 = glm::translate(currentTransform1, move1);
	mRenderObjects.at(0).mTransformMatrix = currentTransform1;

	glm::vec3 scale = glm::vec3(0.05f, 0.05f, 0.05f);
	glm::mat4 modelMatrix = glm::mat4(1.0f);
	glm::mat4 currentTransform = glm::scale(modelMatrix, scale);
	glm::vec3 rotation = glm::vec3(1.0f, 0.0f, 0.0f);
	currentTransform = glm::rotate(currentTransform, glm::radians(180.0f), rotation);
	rotation = glm::vec3(0.0f, 1.0f, 0.0f);
	currentTransform = glm::rotate(currentTransform, glm::radians(180.0f), rotation);
	glm::vec3 move = glm::vec3(-30.0f, 5.0f, 10.0f);
	currentTransform = glm::translate(currentTransform, move);
	mRenderObjects.at(1).mTransformMatrix = currentTransform;

	createSyncObjects();
}

void VulkanRenderer::addRenderObject(RenderObject& renderObj) {
	mRenderObjects.push_back(renderObj);
}

void VulkanRenderer::addRenderObjects(std::vector<RenderObject> renderObjs) {
	for (size_t i = 0; i < renderObjs.size(); i++) 
		mRenderObjects.push_back(renderObjs.at(i));
}

void VulkanRenderer::draw(glm::mat4 cameraViewMatrix) {

	// Wait to make sure the frame is finished. No timeout set for now.
	vkWaitForFences(mDevice->mLogicalDevice, 1, &mInFlightFences[mCurrentFrame], VK_TRUE, std::numeric_limits<uint32_t>::max());
	

	// Since commands are finished executing, I can safely reset the command buffer to begin recording again.
	vkResetCommandBuffer(mMainCommandBuffers[mCurrentFrame], 0);

	// Request Image from the swap chain.
	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(mDevice->mLogicalDevice, mSwapChain->mSwapChain,
		std::numeric_limits<uint32_t>::max(), mImageAvailableSemaphores[mCurrentFrame], VK_NULL_HANDLE, &imageIndex);

	VkCommandBuffer cmd = mMainCommandBuffers[mCurrentFrame];


	// Begin command buffer recording. Using this command buffer once.
	VkCommandBufferBeginInfo cmdBeginInfo{};
	cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;


	vkBeginCommandBuffer(cmd, &cmdBeginInfo);

	// Set clear color.
	VkClearValue clearValue;
	float flash = abs(sin(imageIndex / 120.0f));
	clearValue.color = { {0.0f, 0.0f, flash, 1.0f} };

	VkClearValue depthClear;
	depthClear.depthStencil.depth = 1.f;

	std::array<VkClearValue, 2> clearValues{clearValue, depthClear};

	// Begin the main render pass.
	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = mRenderPass->mRenderPass;
	renderPassInfo.framebuffer = mSwapChain->mSwapChainFrameBuffers[imageIndex]->mFrameBuffer;
	renderPassInfo.renderArea.offset = { 0,0 };
	renderPassInfo.renderArea.extent = mSwapChain->mSwapChainExtent;
	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipeline->mGraphicsPipeline);

	for (size_t i = 0; i < mRenderObjects.size(); i++)
		mRenderObjects.at(i).drawObject(cmd, mGraphicsPipeline->mPipelineLayout, imageIndex);

	vkCmdEndRenderPass(cmd);

	vkEndCommandBuffer(cmd);

	// Update the Uniform Buffers after receiving the current swapchain image.
	for (size_t i = 0; i < mRenderObjects.size(); i++)
		mRenderObjects.at(i).updateUniformBuffers(mSwapChain->mSwapChainExtent, imageIndex, cameraViewMatrix);

	// Queue submission and synchonization is configured through parameters in the VkSubmitInfo struct
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	// First specify which semaphores to wait on before execution begins and in which stages of the pipeilne to wait.
	VkSemaphore waitSemaphores[] = { mImageAvailableSemaphores[mCurrentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	// Next two params specify which command buffers to actually submit for execution.
	// We should submit the command buffer that binds the swap chain image we just acquired as color attachment
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmd;

	// These specify which semaphores to signal once the command buffer have finished execution.
	VkSemaphore signalSemaphores[] = { mRenderFinishedSemaphores[mCurrentFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	vkResetFences(mDevice->mLogicalDevice, 1, &mInFlightFences[mCurrentFrame]);

	// now submit the command buffer to the graphics queue using vkQueueSubmit.
	// It takes and array of VkSubmitInfo structs as arguments for efficiency when the workload is much larger.
	// The last param references an optional fence that will be signaled with the command buffers finish execution.
	// Because I'm using semaphores, I pass VK_NULL_HANDLE.
	// CHANGED to include fences between GPU and CPU
	if (vkQueueSubmit(mDevice->mGraphicsQueue, 1, &submitInfo, mInFlightFences[mCurrentFrame]) != VK_SUCCESS)
		CORE_ERROR("Error: Failed to submit draw command buffer.");

	// Last step of drawing a frame is submitting the result back to the swap chain to have it eventually show up on screen.
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	// Specifies which semaphores to wait on before presentation can happen.
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	// These specify the swap chins to present iamges to and the index of the iamge for each swap chain.
	// This will almost always be a single one.
	VkSwapchainKHR swapChains[] = { mSwapChain->mSwapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;

	result = vkQueuePresentKHR(mDevice->mPresentQueue, &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || mFrameBufferResized) {
		mFrameBufferResized = false;
		//recreateSwapChain();
	}
	else if (result != VK_SUCCESS)
		CORE_ERROR("Failed to present swap chain image in draw frame.");

	// Advance to the next frame
	mCurrentFrame = (mCurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VulkanRenderer::calculateMemoryBudget() {}

void VulkanRenderer::createSyncObjects() {
	// Resize to how many frames I want to be worked on at the end of drawFrame()
	mImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	mRenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	mInFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
	mImagesInFlight.resize(mSwapChain->mSwapChainImages.size(), VK_NULL_HANDLE);

	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	// Create fence info struct
	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	// NEEDED since fences are created in an unsignaled state. It will wait forever if this is not set freezing the program
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	// Loop to build all the semaphores needed so each frame has it's own

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		if (vkCreateSemaphore(mDevice->mLogicalDevice, &semaphoreInfo, nullptr, &mImageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(mDevice->mLogicalDevice, &semaphoreInfo, nullptr, &mRenderFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(mDevice->mLogicalDevice, &fenceInfo, nullptr, &mInFlightFences[i]) != VK_SUCCESS)
			CORE_ERROR("Failed to create synchronization objects for a frame.");
	}
}

void VulkanRenderer::createCommandBuffers() {
	mMainCommandBuffers.resize(mSwapChain->mSwapChainImages.size());

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = mCommandPool->mCommandPool;
	allocInfo.commandBufferCount = (uint32_t)mMainCommandBuffers.size();

	if (vkAllocateCommandBuffers(mDevice->mLogicalDevice, &allocInfo, mMainCommandBuffers.data()) != VK_SUCCESS)
		CORE_ERROR("Error: Failed to allocate main command buffers");
}

void VulkanRenderer::loadRenderObjects() {
	for (size_t i = 0; i < mRenderObjects.size(); i++)
		mRenderObjects.at(i).init(*mCommandPool, static_cast<uint32_t>(mSwapChain->mSwapChainImages.size()));
}

