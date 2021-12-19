#pragma once

#include "../../pch.h"

class VDevice;
class VSurface;

// So far just a command pool for graphics queue family.
// Need to change later to get command pools with different queueFamilyIndex's.
class VCommandPool
{
public:
	VCommandPool(VDevice& device, VSurface surface);
	~VCommandPool();

	VkCommandPool mCommandPool;
	VDevice& mDevice;
};