#pragma once

#include "../../pch.h"
#include "FrameBuffer.h"
#include "Image.h"
#include "ImageView.h"

struct Texture
{
	Texture(std::string texturePath);
	void createTextureImage(
		VkDevice device, 
		VkPhysicalDevice physicalDevice, 
		VkCommandPool commandPool,
		VkQueue graphicsFamily);

	void createTextureImageView(VkDevice device);

	void createTextureSampler(VkPhysicalDevice physicalDevice, VkDevice device);


	std::string mTexturePath;
	VkImage mTextureImage;
	VkImageView mTextureImageView;
	VkDeviceMemory mTextureImageMemory;
	VkSampler mTextureSampler;
};