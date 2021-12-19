#include "../pch.h"
#include "Engine.h"
#include "../Renderer/VulkanRenderer.h"
#include "../Renderer/RenderObject.h"
#define VMA_IMPLEMENTATION
#include "../ThirdParty/vk_mem_alloc.h"
#include <GLFW/glfw3.h>

Engine::Engine(uint32_t width, uint32_t height, std::string title)
	:mWindow(Window(width, height, title)), mRenderer(new VulkanRenderer(&mWindow))
{

}

Engine::~Engine()
{
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{

}

void Engine::init()
{
	RenderObject tmp("Media/Obj/chalet.obj", "Media/Textures/chalet.jpg");
	mRenderer->addRenderObject(tmp);
	tmp = RenderObject("Media/Obj/viking.obj", "Media/Textures/viking.png");
	mRenderer->addRenderObject(tmp);
	mRenderer->init("Test App", "SPX_ENGINE", true);

	// Need to figure out a better way to do this.
	glfwSetKeyCallback(mWindow.getContext(), key_callback);
}

void Engine::run()
{
	while (!glfwWindowShouldClose(mWindow.getContext()))
	{
		createEvents();
		mRenderer->draw();
	}
}

// Update this over time with more events.
void Engine::createEvents()
{
	glfwPollEvents();
}

//void Engine::drawFrame()
//{
//	// This function will preform the following operations:
//		// Aquire an image from the swap chain
//		// Execute the command buffer with that image as attachment in the framebuffer
//		// Return the image to the swap chain for presentation
//		// Each of these events is set in motion using a single function call, but they are executed asynchronously.
//		// The function calls will return before the operations are acually finished and the order of execution is also undefined.
//		// This is unforunate because each of the operations depends on the previous one finishing.
//		// There are two ways of synchornizing the swap chain events: fences and semaphores.
//		// These are both objects that can be used for coordinating operations by having one operation signal and another operation
//		// wait for a fence or semaphore to go from the unsignaled to signaled state.
//		// The difference is that the state of fences can be accessed from your program using calls like vkWaitForFences and semaphores
//		// cannot be. Fences are designed to synchronize your application itself with rendering operation
//		// Semaphores are used to synchronize operatoins within or across command queues.
//		// Semaphores are the best fit because I want to synchronize the queue operations of draw commands and presentation.
//
//		// This makes sure to wait for the frame to be finished
//		// Also takes a time out, using max removes a time out
//		// Unlike semaphores, the fence has to be reset with vkResetFences call
//	vkWaitForFences(
//		mRenderer.mDevice->mLogicalDevice, 
//		1, 
//		&mRenderer.mInFlightFences[mRenderer.mCurrentFrame], 
//		VK_TRUE, 
//		std::numeric_limits<uint32_t>::max());
//
//	// First aquire an image from the swap chain
//	// Swap chain is an extension feature, so must use a function with the vk*KHR naming
//	// First two params are the logical device and swapchain.
//	// Third specifies a timeout in nanoseconds for an image to become available. Using max disables the timeout
//	// Next two specify synchronization objects that are signaled when the presentation engine is finished using an image.
//	// Last param specifies a variable to output the index of the swapchain image that has become available.
//	// Index referes to the VkImage in our mSwapChainImages array.
//	uint32_t imageIndex;
//	// Setting this to a result so I know if the swap chain is out of date. Vulkan will tell me
//	// UINT64_MAX CAN REPLACE THE STD::NUMERIC_LIMITS<UINT64_T>::MAX()
//	VkResult result = vkAcquireNextImageKHR(
//		mRenderer.mDevice->mLogicalDevice, 
//		mRenderer.mSwapChain->mSwapChain,
//		UINT64_MAX,
//		mRenderer.mImageAvailableSemaphores[mRenderer.mCurrentFrame], 
//		VK_NULL_HANDLE, 
//		&imageIndex);
//
//	// Recreate swapchain if it's out of date. Ending the draw frame immediatly so the new swapchain can be made
//	if (result == VK_ERROR_OUT_OF_DATE_KHR)
//	{
//		//recreateSwapChain();
//		return;
//	}
//	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
//	{
//		CORE_ERROR("Failed to acquire swapchain image in draw frame.");
//	}
//
//	if (mRenderer.mImagesInFlight[imageIndex] != VK_NULL_HANDLE)
//		vkWaitForFences(mRenderer.mDevice->mLogicalDevice, 
//			1, 
//			&mRenderer.mImagesInFlight[imageIndex], 
//			VK_TRUE, 
//			std::numeric_limits<uint32_t>::max());
//
//	mRenderer.mImagesInFlight[imageIndex] = mRenderer.mInFlightFences[mRenderer.mCurrentFrame];
//
//	// Update the Uniform buffer AFTER recieving the current swapchain image
//	for (size_t i = 0; i < mRenderer.mModels.size(); i++)
//	{
//		mRenderer.mModels.at(i)->updateModelUniformBuffers(mRenderer.mSwapChain->mSwapChainExtent, imageIndex);
//	}
//	//mRenderer.updateUniformBuffer(imageIndex);
//
//	// Queue submission and synchronization is configured through parameters in the VkSubmitInfo struct
//	VkSubmitInfo submitInfo = {};
//	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
//
//	// First three specify which semaphores to wait on before execution begins and in which stage(s) of the pipeline to wait.
//	VkSemaphore waitSemaphores[] = { mRenderer.mImageAvailableSemaphores[mRenderer.mCurrentFrame] };
//	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
//	submitInfo.waitSemaphoreCount = 1;
//	submitInfo.pWaitSemaphores = waitSemaphores;
//	submitInfo.pWaitDstStageMask = waitStages;
//
//	// Next two params specify which command buffers to actually submit for execution.
//	// We should submit the command buffer that binds the swap chain image we just acquired as color attachment
//	submitInfo.commandBufferCount = 1;
//	submitInfo.pCommandBuffers = &mRenderer.mCommandBuffer->mCommandBuffers[imageIndex];
//
//	// These specifiy which semaphore to signal once the command buffer have finished execution.
//	VkSemaphore signalSemaphores[] = { mRenderer.mRenderFinishedSemaphores[mRenderer.mCurrentFrame] };
//	submitInfo.signalSemaphoreCount = 1;
//	submitInfo.pSignalSemaphores = signalSemaphores;
//
//	vkResetFences(mRenderer.mDevice->mLogicalDevice, 1, &mRenderer.mInFlightFences[mRenderer.mCurrentFrame]);
//
//	// now submit the command buffer to the graphics queue using vkQueueSubmit.
//	// It takes and array of VkSubmitInfo structs as arguments for efficiency when the workload is much larger.
//	// The last param references an optional fence that will be signaled with the command buffers finish execution.
//	// Because I'm using semaphores, I pass VK_NULL_HANDLE.
//	// CHANGED to include fences between GPU and CPU
//	if (vkQueueSubmit(mRenderer.mDevice->mGraphicsQueue, 1, &submitInfo, mRenderer.mInFlightFences[mRenderer.mCurrentFrame]) != VK_SUCCESS)
//		CORE_ERROR("Failed to submit draw command buffer.");
//
//	// Last step of drawing a frame is submitting the result back to the swap chain to have it eventually show up on screen.
//	VkPresentInfoKHR presentInfo = {};
//	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
//	// Specifies which semaphores to wait on before presentation can happen
//	presentInfo.waitSemaphoreCount = 1;
//	presentInfo.pWaitSemaphores = signalSemaphores;
//
//	// These specify the swap chains to present iamges to and teh index of the image for each swap chain.
//	// This will almost be a single one
//	VkSwapchainKHR swapChains[] = { mRenderer.mSwapChain->mSwapChain };
//	presentInfo.swapchainCount = 1;
//	presentInfo.pSwapchains = swapChains;
//	presentInfo.pImageIndices = &imageIndex;
//	// Last param is optional. it allows me to specify an array of result values to check for every
//	// individual swap chain if presentation is successful. Not necessary if I'm only using a single swap chain
//	// becuase I can simply use the return value of the present function
//	// This function submits the requrest to present the image to the swap chain
//	result = vkQueuePresentKHR(mRenderer.mDevice->mPresentQueue, &presentInfo);
//
//	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || mRenderer.mFramedBufferResized)
//	{
//		mRenderer.mFramedBufferResized = false;
//		//recreateSwapChain();
//	}
//	else if (result != VK_SUCCESS)
//	{
//		CORE_ERROR("Failed to present swap chain image in draw frame.");
//	}
//
//	// Advance to the next frame
//	mRenderer.mCurrentFrame = (mRenderer.mCurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
//}
