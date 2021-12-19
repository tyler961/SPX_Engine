#include "VCommandPool.h"
#include "VDevice.h"
#include "VSurface.h"

VCommandPool::VCommandPool(VDevice& device, VSurface surface)
	:mDevice(device)
{
	// Using VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT to allow for resetting of individual command buffers.
	QueueFamilyIndices queueFamilyIndices = VDevice::findQueueFamilies(mDevice.mPhysicalDevice, surface.getSurface());

	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	if (vkCreateCommandPool(mDevice.mLogicalDevice, &poolInfo, nullptr, &mCommandPool) != VK_SUCCESS)
		CORE_ERROR("Failed to create Command Pool");
	else
		CORE_INFO("Command Pool created Successfully.");
}

VCommandPool::~VCommandPool()
{
	//vkDestroyCommandPool(mDevice.mLogicalDevice, mCommandPool, nullptr);
}
