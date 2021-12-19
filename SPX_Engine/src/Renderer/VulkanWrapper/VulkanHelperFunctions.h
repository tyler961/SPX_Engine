#pragma once

#include "../../pch.h"
#include "DataStructures.h"
#include "VDevice.h"
#include "../../ThirdParty/vk_mem_alloc.h"


namespace VHF
{
	struct VulkanHelperFunctions
	{
		static VkFormat findSupportedFormat(
			const std::vector<VkFormat>& candidates,
			VkImageTiling tiling,
			VkFormatFeatureFlags features,
			VkPhysicalDevice device)
		{
			for (VkFormat format : candidates)
			{
				VkFormatProperties prop;
				vkGetPhysicalDeviceFormatProperties(device, format, &prop);

				if (tiling == VK_IMAGE_TILING_LINEAR && (prop.linearTilingFeatures & features) == features)
					return format;
				else if (tiling == VK_IMAGE_TILING_OPTIMAL && (prop.optimalTilingFeatures & features) == features)
					return format;
			}

			CORE_ERROR("Failed to find a supported format.");
		}

		static VkFormat findDepthFormat(VkPhysicalDevice device)
		{
			return findSupportedFormat(
				{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
				VK_FORMAT_D24_UNORM_S8_UINT },
				VK_IMAGE_TILING_OPTIMAL,
				VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT,
				device
			);
		}

		static AllocatedBuffer createBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage, VDevice device)
		{
			VkBufferCreateInfo bufferInfo{};
			bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferInfo.size = allocSize;
			bufferInfo.usage = usage;

			// Let the VMA library know that this data should be writeable by the CPU, but also readable by GPU
			VmaAllocationCreateInfo vmaInfo{};
			vmaInfo.usage = memoryUsage;

			AllocatedBuffer newBuffer;

			if (vmaCreateBuffer(device.mAllocator, &bufferInfo, &vmaInfo, &newBuffer.mBuffer, &newBuffer.mAlloc, nullptr) != VK_SUCCESS)
				CORE_ERROR("Error: Failed to create buffer.");

			return newBuffer;
		}
	};

}
