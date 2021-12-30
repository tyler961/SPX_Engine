#pragma once

#include "../../pch.h"
#include "../../ThirdParty/vk_mem_alloc.h"

// TODO: Add:
// isExtensionSupported
// isLayerSupported
// isSwapChainSupported
// isPresentationSupported
// as static function checks

// TODO: Add getters and make variables private
// TODO: Add transfer queue for data to GPU

class VInstance;

struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;
	//std::optional<uint32_t> transferFamily;

	bool isComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value(); //&& transferFamily.has_value();
	}
};

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentMode;
};


class VDevice {
public:
	VDevice(VkSurfaceKHR surface, VInstance instance);
	
	// Selected a GPU to use
	void pickPhysicalDevice(VInstance instance);
	void createLogicalDevice(VInstance instance);
	void printPhysicalDeviceName();

	static SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
	static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);

	VkPhysicalDevice mPhysicalDevice{ VK_NULL_HANDLE };
	VkDevice mLogicalDevice{ VK_NULL_HANDLE };
	VkSurfaceKHR mSurface{ VK_NULL_HANDLE };

	VkQueue mGraphicsQueue{ VK_NULL_HANDLE };
	VkQueue mPresentQueue{ VK_NULL_HANDLE };
	// TODO: Look up transfer queue and implement it
	// VkQueue mTransferQueue{ VK_NULL_HANDLE };

	// List of required device extensions
	const std::vector<const char*> mDeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	// Vma Info
	VmaAllocator mAllocator{ VK_NULL_HANDLE };


private:
	// Rates all the available devices and then picks the one with the highest score.
	// TODO: Add more things to check to pick the best GPU. For now if its descrete GPU and is suitable it is picked.
	VkPhysicalDevice pickBestPhysicalDevice(std::vector<VkPhysicalDevice> devices);
	int rateDeviceSuitability(VkPhysicalDevice device);
	bool isDeviceSuitable(VkPhysicalDevice device);
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
};