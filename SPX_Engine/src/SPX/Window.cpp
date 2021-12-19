#include "../pch.h"
#include "Window.h"
#include "../Events/Event.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

Window::Window(uint32_t width, uint32_t height, std::string title)
	:mTitle(title), mWidth(width), mHeight(height)
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // Makes sure glfw doesnt create an OpenGL context with the window
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);   // Temporary until I put in swap chain events for resizing.


	mContext = glfwCreateWindow(mWidth, mHeight, mTitle.c_str(), nullptr, nullptr);

	glfwSetWindowUserPointer(mContext, this);

	glfwSetKeyCallback(mContext, [](GLFWwindow* window, int key, int scancode, int action, int mods)
	{

	});
			

	// Callback for when the window is resized so swapchain can be recreated
	// ADD LATER
}

Window::~Window()
{
}

GLFWwindow* Window::getContext()
{
	return mContext;
}
