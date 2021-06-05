#pragma once

#include "../../SPXENGINE/SPX_Engine/SPX_Engine/src/SPX/Core.h"
#include "../../pch.h"
#include "GLFW/glfw3.h"
#include "ValidationLayers.h"

class Renderer
{
public:
	Renderer();
	void createInstance();
	void setupDebug();
	void createSurface();

private:
	ValidationLayers mValLayers;
	
	VkInstance mInstance; // Vulkan Instance
	VkSurfaceKHR mSurface; // Platform independent window object


};