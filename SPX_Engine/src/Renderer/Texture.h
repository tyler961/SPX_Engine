#pragma once

#include "../pch.h"

class VDevice;
class VCommandPool;
class VImage;

class Texture {
public:
	Texture(std::string texturePath, VDevice& device);

	void init(VCommandPool commandPool);
	void createTextureSampler();

	VDevice& mDevice;
	VImage* mTextureImage;
	VkSampler mTextureSampler; // TODO: Change this.

	uint32_t mWidth;
	uint32_t mHeight;
	uint32_t mMipLevels;
	uint32_t mLayerCount;

	VkDescriptorImageInfo mDescriptor;
	std::string mFileLocation;
};