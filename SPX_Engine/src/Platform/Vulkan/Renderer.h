#pragma once

#include "../../SPXENGINE/SPX_Engine/SPX_Engine/src/SPX/Core.h"
#include "../../pch.h"
#include "GLFW/glfw3.h"
#include "ValidationLayers.h"
#include "../../SPXENGINE/SPX_Engine/SPX_Engine/src/SPX/Window.h"
#include "Device.h"
#include "SwapChain.h"
#include "GraphicsPipeline.h"


class Renderer
{
public:
	Renderer(Window* window);
	void createInstance();
	void setupDebug();
	void createSurface();
	void createRenderPass();
	void createDescriptorSetLayout();
	void createCommandPool(); // Maybe move this to command buffer class later.
	VkFormat findDepthFormat();
	VkFormat findSupportedFormat(
		const std::vector<VkFormat>& candidates,
		VkImageTiling tiling, 
		VkFormatFeatureFlags features);


private:
	ValidationLayers mValLayers;
	Device* mDevice;
	SwapChain* mSwapChain;
	GraphicsPipeline* mPipeline;

	VkRenderPass mRenderPass;
	VkDescriptorSetLayout mDescriptorSetLayout;
	VkCommandPool mCommandPool;

	Window* mWindow;

	VkInstance mInstance; // Vulkan Instance
	VkSurfaceKHR mSurface; // Platform independent window object
};