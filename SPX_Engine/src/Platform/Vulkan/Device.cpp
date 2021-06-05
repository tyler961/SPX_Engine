#include "Device.h"

Device::Device(VkSurfaceKHR surface)
	: mSurface(surface), mLogicalDevice(VK_NULL_HANDLE), mPhysicalDevice(VK_NULL_HANDLE)
{
}

void Device::printPhysicalDeviceName()
{
	if (mPhysicalDevice != VK_NULL_HANDLE)
	{
		VkPhysicalDeviceProperties props{};
		vkGetPhysicalDeviceProperties(mPhysicalDevice, &props);
		std::string msg = "Selected GPU: ";
		msg += props.deviceName;
		CORE_TRACE(msg);
	}
}

void Device::pickPhysicalDevice(VkInstance instance)
{
	// List all available GPUs
	SPX_INT deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	if (deviceCount == 0)
		CORE_ERROR("Failed to find any GPUs with Vulkan support.");

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	mPhysicalDevice = pickBestPhysicalDevice(devices);

	// If none are found:
	if (mPhysicalDevice == VK_NULL_HANDLE)
		CORE_ERROR("Failed to find a suitable GPU");
	else
	{
		CORE_INFO("GPU selected successfully.");
		printPhysicalDeviceName();
	}
}

VkPhysicalDevice Device::pickBestPhysicalDevice(std::vector<VkPhysicalDevice> devices)
{
	VkPhysicalDevice selectedDevice;

	std::multimap<int, VkPhysicalDevice> candidates;
	for (const auto& device : devices)
	{
		int score = rateDeviceSuitability(device);
		candidates.insert(std::make_pair(score, device));
	}
	if (candidates.rbegin()->first > 0)
	{
		selectedDevice = candidates.rbegin()->second;
	}
	else
	{
		CORE_ERROR("Failed to find a suitable GPU.");
		selectedDevice = VK_NULL_HANDLE;
	}

	return selectedDevice;
}


// Scores it based on its abilities and if it supports certain features, extensions and swap chain
int Device::rateDeviceSuitability(VkPhysicalDevice device)
{
	int score = 0;

	// Discrete GPUs have a significant advantage
	VkPhysicalDeviceFeatures feats{};
	vkGetPhysicalDeviceFeatures(device, &feats);
	VkPhysicalDeviceProperties props{};
	vkGetPhysicalDeviceProperties(device, &props);

	if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		score += 1000;

	score += props.limits.maxImageDimension2D;

	// Can't function without a geometry shader.
	if (!feats.geometryShader)
		return 0;
	else if (!isDeviceSuitable(device))
		return 0;

	return score;
}

bool Device::isDeviceSuitable(VkPhysicalDevice device)
{
	// Once a device has been selected, make sure it supports the required queue families.
	QueueFamilyIndices indices = findQueueFamilies(device, mSurface);

	bool extensionsSupported = checkDeviceExtensionSupport(device);

	bool swapChainAdequate = false;
	if (extensionsSupported)
	{
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device, mSurface);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentMode.empty();
	}

	// Checking for Anisotopy
	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

	return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

bool Device::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
	SPX_INT extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExt(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExt.data());

	std::set<std::string> requiredExensions(mDeviceExtensions.begin(), mDeviceExtensions.end());

	for (const auto& extension : availableExt)
		requiredExensions.erase(extension.extensionName);

	// If all of the extensions I want have been found this will be empty resulting in a true being returned.
	return requiredExensions.empty();
}

SwapChainSupportDetails Device::querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

	SPX_INT formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

	if (formatCount != 0)
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
	}

	SPX_INT presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

	if (presentModeCount != 0)
	{
		details.presentMode.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentMode.data());
	}

	return details;
}



QueueFamilyIndices Device::findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	QueueFamilyIndices indices;
	// Gets queue family info supported by the device
	SPX_INT queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies)
	{
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			indices.graphicsFamily = i;

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

		if (presentSupport)
			indices.presentFamily = i;

		if (indices.isComplete())
			break;

		i++;
	}

	return indices;
}




void Device::createLogicalDevice(ValidationLayers valLayers)
{
	QueueFamilyIndices indices = findQueueFamilies(mPhysicalDevice, mSurface);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<SPX_INT> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	float queuePriority = 1.0f;

	// Need to have multiple VkDeviceQueueCreateInfos structs to create a queue from both families.
	// Elegant way to do that is to create a set of all the unique queue families that are necessary for the required queues.
	for (SPX_INT queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	// Adding Anisotrophy
	// isDeviceSuitable checks and makes sure that this is supported
	VkPhysicalDeviceFeatures feats{};
	feats.samplerAnisotropy = VK_TRUE;

	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = static_cast<SPX_INT>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.pEnabledFeatures = &feats;

	createInfo.enabledExtensionCount = static_cast<SPX_INT>(mDeviceExtensions.size());
	createInfo.ppEnabledExtensionNames = mDeviceExtensions.data();

	if (valLayers.mEnableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<SPX_INT>(valLayers.mValidationLayers.size());
		createInfo.ppEnabledLayerNames = valLayers.mValidationLayers.data();
	}
	else
		createInfo.enabledLayerCount = 0;

	// Now I can create the logical device
	// I need the physical device to interface with, the queue and usage info I just specified, the optional allocation
	// callback pointer and a pointer to a variable to store the logcial device handle in.
	if (vkCreateDevice(mPhysicalDevice, &createInfo, nullptr, &mLogicalDevice) != VK_SUCCESS)
		CORE_ERROR("Failed to create logical device.");
	else
		CORE_INFO("Logcial Device created successfully");

	// Retrieves handle for queue. Parameters are logical device, queue family, queue index and pointer 
	// to the variable to store the handle in. Bc I'm only creating a single queu from this family, I can use index 0.
	vkGetDeviceQueue(mLogicalDevice, indices.graphicsFamily.value(), 0, &mGraphicsQueue);
	vkGetDeviceQueue(mLogicalDevice, indices.presentFamily.value(), 0, &mPresentQueue);
}


