#include "VSwapChain.h"
#include "VDevice.h"
#include "../../SPX/Window.h"
#include "VulkanHelperFunctions.h"
#include "VImage.h"
#include "VRenderPass.h"
#include "VFrameBuffer.h"
#include <GLFW/glfw3.h>

VSwapChain::VSwapChain(VDevice& device, Window* window)
	: mDevice(device) {
	SwapChainSupportDetails swapChainSupport = VDevice::querySwapChainSupport(device.mPhysicalDevice, device.mSurface);

	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentMode);
	mSwapChainExtent = chooseSwapExtent(swapChainSupport.capabilities, window);

	// Min number of images it takes for swap chain to function
	// Adding +1 is recommeneded to help reduce the amout of times that I have to wait for the driver
	// to complete internal operations before I can aquire another iamge to render to.
	mSwapChainImageCount = swapChainSupport.capabilities.minImageCount + 1;

	// Need to make sure that by adding the 1 I'm not exceeding the max number of images allowed.
	if (swapChainSupport.capabilities.maxImageCount > 0 && mSwapChainImageCount > swapChainSupport.capabilities.maxImageCount)
		mSwapChainImageCount = swapChainSupport.capabilities.maxImageCount;


	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = device.mSurface;

	// After specifying the surface the swapchain should be tied to, the details of the swapchain are specified.
	// Image Array layers specifies the amount of layers each image consists of.
	// That is always 1 unless I am developing a steroscopic 3D application.
	// imageUsage bit field specifies what kind of operations I'll be using the iamges in the swap chain for.
	// In this example I'm going to render directly to them, which means that they're a color attachment.
	// it is also possible that I'll renderimages to a seperate image first to preform operations like post-processing.
	// To do that I might use VK_IMAGE_USAGE_TRANSFER_DST_BIT instead and use memory operation to transfer the rendered image to the swap chain image.
	createInfo.minImageCount = mSwapChainImageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = mSwapChainExtent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	// Next I need to specify how to handle swap chain images that will be used across multiple queue families.
	// That will be the case in our application if the graphics queue family is different from the presentation queue.
	// I'll be drawing on the images in the swap chain from the graphics queue and then submitting them on the presentation queue.
	// There are two ways to handle images that are accessed from different queues:
	// VK_SHARING_MODE_EXCLUISIVE: An image is owned by one queue family at a time and ownership must be explicitly
	// transfered before using it in another queu family. THIS OPTION OFFERS THE BEST PREFORMANCE
	// VK_SHARING_MODE_CONCURRENT: Images can be used across multiple queue families without explict ownership transfers.
	QueueFamilyIndices indices = VDevice::findQueueFamilies(device.mPhysicalDevice, device.mSurface);
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	if (indices.graphicsFamily != indices.presentFamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0; // Optional
		createInfo.pQueueFamilyIndices = nullptr; // Optional
	}

	// If the queue families differe, then I'll be using the concurrent mode in this example to avoid having to
	// do the ownership chapters.
	// Concurrent mode requires me to specify in advance between which queu families ownership will be shared using
	// the queueFamilyIndexCount and pQueueFamilyIndices prameters.
	// If the graphics queue family and presention queu family are the same, which will be the case on most hardware, then
	// I should stick to excluive mode, bc concurrent mode requires me to specify at least two distint queu families.
	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;

	// I can specify that a certain transform should be applied to images in the swapchain if it is supported (supportedTransforms in capabilities)
	// Such as a 90 degree rotation or horizontal flip. To specify that I do not want any transforms, I specify:
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	// The compositeAlpha field specifies if the alpha channel should be used for blending with other windows in the window system.
	// You'll almost always want to simply ignore the alpha channel.

	// If the clipped member is set to VK_TRUE then that means I dont care about the color of the pixels that are obscured,
	// for example because another window is in front of them. Unless I really need to be able to read those pixels,
	// then to get the best preformance by enabling clipping
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;

	// This leaves the last field to fill out.
	// With Vulkan it's possible that my swapchain becomes invalid or unoptimized while my applicaiton is running.
	// Example, the window is resized. In that case the swapchain actually needs to be recreated from scratch
	// and a reference to the old one must be specified in this field.
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	// Parameters are logical device, swapchain creation info, optional custom allocator, and a pointer to the variable to store the handle in
	if (vkCreateSwapchainKHR(device.mLogicalDevice, &createInfo, nullptr, &mSwapChain) != VK_SUCCESS)
		CORE_ERROR("ERROR: Failed to create swap chain!");
	else
		CORE_INFO("Swap Chain created successfully.");

	// Get VkImage handles
	vkGetSwapchainImagesKHR(device.mLogicalDevice, mSwapChain, &mSwapChainImageCount, nullptr);
	mSwapChainImages.resize(mSwapChainImageCount);
	vkGetSwapchainImagesKHR(device.mLogicalDevice, mSwapChain, &mSwapChainImageCount, mSwapChainImages.data());

	// Get references to the format and extent created
	mSwapChainImageFormat = surfaceFormat.format;

	// Create image views here
	VkImageViewCreateInfo imageViewCreateInfo{};
	imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	// Allows me to treat images as 1D textures, 2D textures or 3D textures and cube maps.
	imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewCreateInfo.format = surfaceFormat.format;
	// TODO: Make this a dynamic choice by the user.
	imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	imageViewCreateInfo.subresourceRange.levelCount = 1;
	imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	imageViewCreateInfo.subresourceRange.layerCount = 1;
	
	mSwapChainImageViews.resize(mSwapChainImageCount);

	for (uint32_t i = 0; i < mSwapChainImageCount; i++) {
		imageViewCreateInfo.image = mSwapChainImages.at(i);

		if (vkCreateImageView(device.mLogicalDevice, &imageViewCreateInfo, nullptr, &mSwapChainImageViews.at(i)) != VK_SUCCESS)
			CORE_ERROR("Failed to create Image view in Swap Chain.");
	}
}

void VSwapChain::createSwapChainFrameBuffers(VRenderPass renderPass) {
	createDepthResources();

	mSwapChainFrameBuffers.resize(mSwapChainImageViews.size());

	for (size_t i = 0; i < mSwapChainImageViews.size(); i++) {
		VFrameBuffer* tmp = new VFrameBuffer(*this, renderPass, mDevice, static_cast<uint32_t>(i));
		mSwapChainFrameBuffers.at(i) = tmp;
	}
}

void VSwapChain::createDepthResources() {
	VkFormat depthFormat = VHF::VulkanHelperFunctions::findDepthFormat(mDevice.mPhysicalDevice);

	mDepthImage = new VImage(mDevice, depthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 
		VK_IMAGE_ASPECT_DEPTH_BIT, VK_SAMPLE_COUNT_1_BIT, "bleh", mSwapChainExtent);
}


VkSurfaceFormatKHR VSwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
	for (const auto& availableFormat : availableFormats) {
		// Checks if format and color space are correct. This is the preferred format and colorspace
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			return availableFormat;
	}

	return availableFormats[0];
}


VkPresentModeKHR VSwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
	for (const auto& availablePresentMode : availablePresentModes) {
		// This checks for the preferred method which is triple buffering queue of images to be presented.
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			return availablePresentMode;
		// This mode sends the image even if it isnt ready instead of having the program wait. Can produce screen tearing.
		else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
			return availablePresentMode;
	}

	// This is supported by all graphics cards, so defaults to this.
	// It is a double buffer and the program waits for the image to be done before sending it.
	// Not optimal, but not bad either
	return VK_PRESENT_MODE_FIFO_KHR;
}


// The swapchain extent is the resolution of the swap chain image
// It is almost exactly the resolution of the window that is being drawn to.
VkExtent2D VSwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& cap, Window* window) {
	if (cap.currentExtent.width != UINT64_MAX)
		return cap.currentExtent;

	else {
		int width, height;
		glfwGetFramebufferSize(window->getContext(), &width, &height);

		VkExtent2D actualExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

		actualExtent.width = std::max(cap.minImageExtent.width,
			std::min(cap.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(cap.minImageExtent.height,
			std::min(cap.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}
