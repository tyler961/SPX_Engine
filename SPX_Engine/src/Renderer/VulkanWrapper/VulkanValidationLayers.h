#pragma once

#include "../../pch.h"


class VulkanValidationLayers
{
public:
	VulkanValidationLayers(bool enable);
	// TODO: Have this call DestroyDebugUtilsMessengerEXT. Have to give this class a reference to the instance to do this.
	~VulkanValidationLayers();

	bool checkValidationLayerSupport();
	// Called by renderer.
	void init(VkInstance instance);

	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* pDebugMessenger);

	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
		const VkAllocationCallbacks* pAllocator);

	// Sets what types of messages to recieve and what severities.
	// TODO: Make this setable in the app? Give more control to the user rather than forcing these types.
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& info);


	// Sets the way to display Validation layers. 
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);

	// TODO : Move to private at a later date.
	const std::vector<const char*> mValidationLayers = { "VK_LAYER_KHRONOS_validation" };
	VkDebugUtilsMessengerEXT mDebugMessenger;
	bool mEnableValidationLayers{ true };

private:
};