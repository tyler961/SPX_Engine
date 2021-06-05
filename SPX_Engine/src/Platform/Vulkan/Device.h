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

	VkPhysicalDevice mPhysicalDevice;
	VkDevice mDevice;
	VkSurfaceKHR mSurface;

	VkQueue mGraphicsQueue;
	VkQueue mPresentQueue;

	// List of required device extensions
	const std::vector<const char*> mDeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	void printPhysicalDeviceName();

	void pickPhysicalDevice(VkInstance instance);
	VkPhysicalDevice pickBestPhysicalDevice(std::vector<VkPhysicalDevice> devices);
	int rateDeviceSuitability(VkPhysicalDevice device);
	bool isDeviceSuitable(VkPhysicalDevice device);
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);


	void createLogicalDevice(ValidationLayers valLayers);

private:
};