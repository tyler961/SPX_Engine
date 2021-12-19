#pragma once

#include "../../pch.h"

class VDevice;
class VSwapChain;
class VRenderPass;

// For now this is closely tied to only the Swapchain. Not sure if I'll ever need it somewhere else too. So for now it is how it is.

class VFrameBuffer
{
public:
	VFrameBuffer(VSwapChain swapchain, VRenderPass renderPass, VDevice& device, uint32_t index);
	~VFrameBuffer();



	VkFramebuffer mFrameBuffer{ VK_NULL_HANDLE };

private:
	VDevice& mDevice;
};