#include "Renderer.h"

Renderer::Renderer(Window* window)
	: mWindow(window), mDevice(nullptr)
{
	createInstance();
	setupDebug();
	createSurface();
	mDevice = new Device(mSurface);
	mDevice->pickPhysicalDevice(mInstance);
	mDevice->createLogicalDevice(mValLayers);
	mSwapChain = new SwapChain(mDevice->mPhysicalDevice, mDevice->mLogicalDevice, mSurface);
	mSwapChain->createSwapChain(mWindow);

}

void Renderer::createInstance()
{
	if (mValLayers.mEnableValidationLayers && !mValLayers.checkValidationLayerSupport())
		CORE_ERROR("Validation layers requested, but not available.");

	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "SPX Engine"; // TODO Change to apply to the application
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0); // Change when farther along
	appInfo.pEngineName = "SPX Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0); // Change when farther along
	appInfo.apiVersion = VK_API_VERSION_1_0;


	// Get extension info for the glfwWindow
	SPX_INT glfwExtensionsCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionsCount);
	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionsCount);

	// IF I DONT DO THIS DEBUG DIES AND WONT LOAD
	if (mValLayers.mEnableValidationLayers)
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);


	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledExtensionCount = static_cast<SPX_INT>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	// Doing this here lets validation layers to be used during VkCreateInstance and VkDestroyInstance
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
	if (mValLayers.mEnableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<SPX_INT>(mValLayers.mValidationLayers.size());
		createInfo.ppEnabledLayerNames = mValLayers.mValidationLayers.data();
		
		mValLayers.populateDebugMessengerCreateInfo(debugCreateInfo);
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
		CORE_INFO("Vulkan Instance created successfully.");
}

void Renderer::setupDebug()
{
	mValLayers.setupDebugMessenger(mInstance);
}

void Renderer::createSurface()
{
	// Link the window from GLFW to vulkan
	if (glfwCreateWindowSurface(mInstance, mWindow->getContext(), nullptr, &mSurface) != VK_SUCCESS)
		CORE_ERROR("Failed to create Vulkan Surface.");
	else
		CORE_INFO("Vulkan Surface created successfully.");
}

