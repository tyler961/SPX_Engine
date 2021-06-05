#pragma once
#include "../../pch.h"
#include "ValidationLayers.h"


struct QueueFamilyIndices
{
	std::optional<SPX_INT> graphicsFamily;
	std::optional<SPX_INT> presentFamily;

	bool isComplete()
	{
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentMode;
};

class Device
{
public:
	Device(VkSurfaceKHR surface);

	void pickPhysicalDevice(VkInstance instance);
	void createLogicalDevice(ValidationLayers valLayers);
	void printPhysicalDeviceName();

	static SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
	static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);


	VkPhysicalDevice mPhysicalDevice;
	VkDevice mLogicalDevice;
	VkSurfaceKHR mSurface;

	VkQueue mGraphicsQueue;
	VkQueue mPresentQueue;

	// List of required device extensions
	const std::vector<const char*> mDeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

private:
	VkPhysicalDevice pickBestPhysicalDevice(std::vector<VkPhysicalDevice> devices);
	int rateDeviceSuitability(VkPhysicalDevice device);
	bool isDeviceSuitable(VkPhysicalDevice device);
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
};