#include "VulkanValidationLayers.h"

VulkanValidationLayers::VulkanValidationLayers(bool enable)
	:mEnableValidationLayers(enable) {}

VulkanValidationLayers::~VulkanValidationLayers() {}


bool VulkanValidationLayers::checkValidationLayerSupport() {
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : mValidationLayers) {
		bool layerFound = false;
		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) {
			return false;
		}
	}

	return true;
}


void VulkanValidationLayers::init(VkInstance instance) {
	if (!mEnableValidationLayers) return;

	VkDebugUtilsMessengerCreateInfoEXT info;
	populateDebugMessengerCreateInfo(info);

	if (CreateDebugUtilsMessengerEXT(instance, &info, nullptr, &mDebugMessenger) != VK_SUCCESS)
		CORE_ERROR("Failed to setup debug messenger.");
	else
		CORE_INFO("Debug Messenger setup successfully.");
}


VkResult VulkanValidationLayers::CreateDebugUtilsMessengerEXT(
	VkInstance instance, 
	const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, 
	const VkAllocationCallbacks* pAllocator, 
	VkDebugUtilsMessengerEXT* pDebugMessenger) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)
		vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}


void VulkanValidationLayers::DestroyDebugUtilsMessengerEXT(
	VkInstance instance, 
	VkDebugUtilsMessengerEXT debugMessenger, 
	const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)
		vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

	if (func != nullptr)
		func(instance, debugMessenger, pAllocator);
}


void VulkanValidationLayers::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& info) {
	// Creates the debug create info struct
	info = {};
	info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

	info.messageSeverity =
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

	info.messageType =
		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;

	info.pfnUserCallback = debugCallback;
}


// TODO: Work on making the messages a little more clean looking.
VKAPI_ATTR VkBool32 VKAPI_CALL VulkanValidationLayers::debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, 
	void* pUserData) {
	if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
		std::cout << std::endl;
		CORE_WARN("----------------------------------------------START----------------------------------------------");
		CORE_WARN(pCallbackData->pMessage);
		CORE_WARN("-----------------------------------------------END-----------------------------------------------");
		std::cout << std::endl;
	}
	else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
		std::cout << std::endl;
		CORE_ERROR("----------------------------------------------START----------------------------------------------");
		CORE_ERROR(pCallbackData->pMessage);
		CORE_ERROR("-----------------------------------------------END-----------------------------------------------");
		std::cout << std::endl;
	}
	else {
		std::cout << std::endl;
		CORE_INFO("----------------------------------------------START----------------------------------------------");
		CORE_INFO(pCallbackData->pMessage);
		CORE_INFO("-----------------------------------------------END-----------------------------------------------");
		std::cout << std::endl;
	}

	return VK_FALSE;
}
