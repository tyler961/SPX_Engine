#include "ImageView.h"

ImageView::ImageView()
{
}

void ImageView::createImageViews(std::vector<VkImage>& swapChainImages, 
	std::vector<VkImageView>& swapChainImageViews, 
	VkFormat swapChainImageFormat, 
	VkExtent2D swapChainExtent,
	VkDevice device)
{
	swapChainImageViews.resize(swapChainImages.size());

	for (size_t i = 0; i < swapChainImages.size(); i++)
		swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, device);
}

VkImageView ImageView::createImageView(
	VkImage image, 
	VkFormat format, 
	VkImageAspectFlags aspectFlags, 
	VkDevice device)
{
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	// Allows me to treat images as 1D textures, 2D textures or 3D textures and cube maps.
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = aspectFlags;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	VkImageView imageView;
	if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
		CORE_ERROR("Failed to create Image View.");

	return imageView;
}
