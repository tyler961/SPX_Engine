#pragma once

#include "../../SPXENGINE/SPX_Engine/SPX_Engine/src/SPX/Core.h"
#include "../../pch.h"
#include "GLFW/glfw3.h"
#include "ValidationLayers.h"
#include "../../SPXENGINE/SPX_Engine/SPX_Engine/src/SPX/Window.h"
#include "Device.h"


class Renderer
{
public:
	Renderer(Window* window);
	void createInstance();
	void setupDebug();
	void createSurface();


private:
	ValidationLayers mValLayers;
	// Pointer because in constructor I need mSurface created first before this is created.
	// It won't let me set it again after its creation. Not sure the work around besides a pointer.
	Device* mDevice;

	Window* mWindow;


	VkInstance mInstance; // Vulkan Instance
	VkSurfaceKHR mSurface; // Platform independent window object


};