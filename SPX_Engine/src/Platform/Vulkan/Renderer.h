#pragma once

#include "../../SPXENGINE/SPX_Engine/SPX_Engine/src/SPX/Core.h"
#include "../../pch.h"
#include "GLFW/glfw3.h"
#include "ValidationLayers.h"
#include "../../SPXENGINE/SPX_Engine/SPX_Engine/src/SPX/Window.h"
#include "Device.h"
#include "SwapChain.h"


class Renderer
{
public:
	Renderer(Window* window);
	void createInstance();
	void setupDebug();
	void createSurface();


private:
	ValidationLayers mValLayers;
	Device* mDevice;
	SwapChain* mSwapChain;

	Window* mWindow;

	VkInstance mInstance; // Vulkan Instance
	VkSurfaceKHR mSurface; // Platform independent window object
};