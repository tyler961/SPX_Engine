#include "VSurface.h"

#include "../../SPX/Window.h"
#include <GLFW/glfw3.h>

VSurface::VSurface(VkInstance instance, Window* window)
	:mInstance(instance)
{
	if (glfwCreateWindowSurface(mInstance, window->getContext(), nullptr, &mSurface) != VK_SUCCESS)
		CORE_ERROR("ERROR: Failed to create Vulkan Surface.");
	else
		CORE_INFO("Created Vulkan Surface successfully.");
}

VSurface::~VSurface()
{
	//vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
}
