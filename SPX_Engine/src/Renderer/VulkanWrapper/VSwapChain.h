#pragma once

#include "../../pch.h"

// ******************************************************************************************************************************
//															SWAP CHAIN
// The swap chain is essentially a queue of images that are waiting to be presented to the screen. The application will aquire
// an image to draw to it, then return it to the queue.
// 
// ******************************************************************************************************************************

#include "VFrameBuffer.h"

class Window;
class VImage;
class VRenderPass;
class VDevice;

class VSwapChain {
public:
	VSwapChain(VDevice& device, Window* window);

	// THIS HAS TO BE CALLED AFTER PIPELINE IS DONE BEING CREATED.
	void createSwapChainFrameBuffers(VRenderPass renderPass);

	void createDepthResources();

	VDevice& mDevice;

	VkSwapchainKHR mSwapChain;
	std::vector<VkImage> mSwapChainImages;
	VkFormat mSwapChainImageFormat;
	VkExtent2D mSwapChainExtent;
	std::vector<VkImageView> mSwapChainImageViews;

	VImage* mDepthImage{ nullptr };

	uint32_t mSwapChainImageCount;

	std::vector<VFrameBuffer*> mSwapChainFrameBuffers;

private:
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& cap, Window* window);
};