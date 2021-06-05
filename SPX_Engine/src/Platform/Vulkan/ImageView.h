#pragma once

// ******************************************************************************************************************************
//														Image Views
// Image views are kind of like a lens into a VkImage. They are subparts, such as representing a single mip level,
// small region of the altas or anything inbetween or beyond. Lets access the image without needing to work on the image
// itself that much.
// 
// From the Vulkan Tutorial on ImageViews:
// To use any VkImage, including those in the swap chain, in the render pipeline we have to create a VkImageView object.
// An image view is quite literally a view into an image. It describes how to access the image and which part of
// the image to access, for example if it should be treated as a 2D texture depth texture without any mipmapping levels.
// 
// 
// 
// SEQUENCE:
// VkMemory:        Is a sequence of N bytes im memory.
// VkImage:         Object adds to it, such as information about the format (so I can address by texels instead of bytes)
// VkImageView:     Object helps select only part (array or mip) of the VkImage (like stringView). Also can help match to
//			        some imcompatiable interface (by type casting format).
// VkFrameBuffer:   binds a VkImageView with an attachment.
// VkRenderPass:    defines which attachment will be drawn into.
// 
// ******************************************************************************************************************************

#include "../../pch.h"


class ImageView
{
public:
	ImageView();

	static void createImageViews(
		std::vector<VkImage>& swapChainImages, 
		std::vector<VkImageView>& swapChainImageViews,
		VkFormat swapChainImageFormat,
		VkExtent2D swapChainExtent,
		VkDevice device
		);

	static VkImageView createImageView(
		VkImage image, 
		VkFormat format, 
		VkImageAspectFlags aspectFlags, 
		VkDevice device);
};