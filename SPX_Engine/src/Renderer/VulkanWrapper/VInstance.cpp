#include "VInstance.h"
#include "VulkanValidationLayers.h"
#include <GLFW/glfw3.h>
#include "../../SPX/Core.h"



VInstance::VInstance(std::string& appName, std::string& engineName, bool enableValidationLayers)
	:mValLayers(new VulkanValidationLayers(enableValidationLayers))
{
	if (enableValidationLayers && !mValLayers->checkValidationLayerSupport())
		CORE_ERROR("ERROR: Validation layers requested, but they are not supported.");

	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "SPX Engine"; // TODO Change to apply to the application
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0); // Change when farther along
	appInfo.pEngineName = "SPX Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0); // Change when farther along
	appInfo.apiVersion = VK_API_VERSION_1_0;


	// Get extension info for the glfwWindow
	uint32_t glfwExtensionsCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionsCount);
	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionsCount);

	// IF I DONT DO THIS DEBUG DIES AND WONT LOAD
	if (enableValidationLayers)
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);


	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	// Doing this here lets validation layers to be used during VkCreateInstance and VkDestroyInstance
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
	if (enableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(mValLayers->mValidationLayers.size());
		createInfo.ppEnabledLayerNames = mValLayers->mValidationLayers.data();

		mValLayers->populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
	else
	{
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
	}

	if (vkCreateInstance(&createInfo, nullptr, &mInstance) != VK_SUCCESS)
		CORE_ERROR("Failed to create Vulkan Instance.");
	else
	{
		CORE_INFO("Vulkan Instance created successfully.");
		mValLayers->init(mInstance);
	}
}

VInstance::~VInstance()
{
}
