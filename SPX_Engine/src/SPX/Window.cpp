#include "../pch.h"
#include "Window.h"
#include "../Events/Event.h"
#include <iostream>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

Window::Window(uint32_t width, uint32_t height, std::string title)
	:mTitle(title), mWidth(width), mHeight(height) {
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // Makes sure glfw doesnt create an OpenGL context with the window
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);   // Temporary until I put in swap chain events for resizing.

	mContext = glfwCreateWindow(mWidth, mHeight, mTitle.c_str(), nullptr, nullptr);

	glfwSetWindowUserPointer(mContext, this);

	// Setup the function I want to be used as a callback for key events. 
	// TODO:: Have this go straight to KeyEvent to create the event there.
	auto func = [](GLFWwindow* w, int key, int scancode, int action, int mods) {
		static_cast<Window*>(glfwGetWindowUserPointer(w))->createKeyboardEvent(key, action);
	};

	glfwSetKeyCallback(mContext, func);
}

Window::~Window() {}

void Window::createKeyboardEvent(int key, int action) {
	if (action == GLFW_PRESS) {
		mEventsQueue.push_back(new Event(key, EventType::KeyPressed, EventCategoryKeyboard));
	}
	if (action == GLFW_RELEASE) {
		mEventsQueue.push_back(new Event(key, EventType::KeyReleased, EventCategoryKeyboard));
	}
}

void Window::pollEvents() {
	glfwPollEvents();	
}

GLFWwindow* Window::getContext() {
	return mContext;
}
