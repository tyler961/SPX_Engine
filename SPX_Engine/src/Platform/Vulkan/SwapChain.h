#pragma once

#include "../../pch.h"
#include "Device.h" // Allow me to use the swapchain support functionailty
#include "../../SPXENGINE/SPX_Engine/SPX_Engine/src/SPX/Window.h"
#include "ImageView.h"


// ******************************************************************************************************************************
//															SWAP CHAIN
// The swap chain is essentially a queue of images that are waiting to be presented to the screen. The application will aquire
// an image to draw to it, then return it to the queue.
// 
// ******************************************************************************************************************************



class SwapChain
{
public:
	SwapChain(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface);
	void createSwapChain(Window* window);
	void createSwapChaimImageViews(VkDevice device);

	VkPhysicalDevice mPhysicalDevice;
	VkDevice mDevice;
	VkSurfaceKHR mSurface;

	VkSwapchainKHR mSwapChain;
	std::vector<VkImage> mSwapChainImages;
	VkFormat mSwapChainImageFormat;
	VkExtent2D mSwapChainExtent;
	std::vector<VkImageView> mSwapChainImageViews;

private:
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& cap, Window* window);
};