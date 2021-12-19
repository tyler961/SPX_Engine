#pragma once

#include "../../pch.h"

class VDevice;
class VSwapChain;

class VRenderPass
{
public:
	// TODO: Don't hardcode the attachments and dependencies and subpass. Use the variables attached to it.
	// This will allow me to have depth stencil or not among MANY other options. 
	VRenderPass(
		const VDevice& device,
		//const std::vector<VkAttachmentDescription>& attachments,
		//const std::vector<VkSubpassDependency>& dependencies,
		//VkSubpassDescription subpassDescription,
		const std::string& name,
		VSwapChain swapChain);

	~VRenderPass();



	// TODO: Make private and use getter functions later.
	const VDevice& mDevice;
	VkRenderPass mRenderPass{ VK_NULL_HANDLE };
	std::string mName; // ?? Get rid of?

private:
};