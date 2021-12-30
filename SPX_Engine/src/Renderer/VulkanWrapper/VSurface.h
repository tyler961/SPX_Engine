#pragma once

#include "../../pch.h"

class Window;

class VSurface
{
public:
	VSurface(VkInstance instance, Window* window);
	~VSurface();

	VkSurfaceKHR getSurface() const { return mSurface; }

private:
	VkInstance mInstance{ VK_NULL_HANDLE };
	VkSurfaceKHR mSurface{ VK_NULL_HANDLE };
};