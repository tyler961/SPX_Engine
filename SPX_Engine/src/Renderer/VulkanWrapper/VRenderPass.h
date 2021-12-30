#pragma once

#include "../../pch.h"

class VDevice;
class VSwapChain;

class VRenderPass
{
public:
	VRenderPass(
		const VDevice& device,
		const std::string& name,
		VSwapChain swapChain);

	~VRenderPass();

	// TODO: Make private and use getter functions later.
	const VDevice& mDevice;
	VkRenderPass mRenderPass{ VK_NULL_HANDLE };
	std::string mName; // ?? Get rid of?
};