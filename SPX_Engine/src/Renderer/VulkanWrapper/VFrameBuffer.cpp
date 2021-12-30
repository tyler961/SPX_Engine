#include "VFrameBuffer.h"
#include "VSwapChain.h"
#include "VDevice.h"
#include "VImage.h"
#include "VRenderPass.h"


VFrameBuffer::VFrameBuffer(VSwapChain swapchain, VRenderPass renderPass, VDevice& device, uint32_t index)
	:mDevice(device), mFrameBuffer(VK_NULL_HANDLE) {
	std::array<VkImageView, 2> attachments = { swapchain.mSwapChainImageViews[index], swapchain.mDepthImage->mImageView };

	VkFramebufferCreateInfo framebufferInfo = {};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;

	// Specify which render pass the frame buffer needs to be compatible with, so they roughly use the same number of attachments
	framebufferInfo.renderPass = renderPass.mRenderPass;

	// Attachmentcount and pAttachments specify the VkImageView objects that should be bound to the respective attachment
	// descriptions in the render pass pAttachment array
	framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	framebufferInfo.pAttachments = attachments.data();

	// Width and height are self-explanatory and layers refers to the number of laying in image arrays.
	// Since the swap chain images are single images, the number of layers is 1
	framebufferInfo.width = swapchain.mSwapChainExtent.width;
	framebufferInfo.height = swapchain.mSwapChainExtent.height;
	framebufferInfo.layers = 1;

	if (vkCreateFramebuffer(device.mLogicalDevice, &framebufferInfo, nullptr, &mFrameBuffer) != VK_SUCCESS)
		CORE_ERROR("Failed to create framebuffer.");
}

VFrameBuffer::~VFrameBuffer() {
	vkDestroyFramebuffer(mDevice.mLogicalDevice, mFrameBuffer, nullptr);
}
