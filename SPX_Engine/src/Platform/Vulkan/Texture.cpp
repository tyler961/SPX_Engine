#include "Texture.h"
#define STB_IMAGE_IMPLEMENTATION

#include "../../stb_image/stb_image.h"

Texture::Texture(std::string texturePath)
	:mTexturePath(texturePath)
{
}

void Texture::createTextureImage(
	VkDevice device, 
	VkPhysicalDevice physicalDevice,
	VkCommandPool commandPool,
	VkQueue graphicsFamily)
{
	// LATER REMOVE NEED FOR STB_IMAGE.
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load(mTexturePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	VkDeviceSize imageSize = texWidth * texHeight * 4;

	if (!pixels)
		CORE_ERROR("Failed to load texture image.");


	// Not 100% sure why this is needed.
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	FrameBuffer::createBuffer(
		imageSize, 
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer, 
		stagingBufferMemory,
		physicalDevice,
		device);

	void* data;
	vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(device, stagingBufferMemory);

	// Clean up pixel array now
	stbi_image_free(pixels);


	Image::createImage(
		texWidth, texHeight, 
		VK_FORMAT_R8G8B8A8_SRGB, 
		VK_IMAGE_TILING_OPTIMAL, 
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
		mTextureImage, mTextureImageMemory,
		device, physicalDevice);

	Image::transitionImageLayout(
		mTextureImage, 
		VK_FORMAT_R8G8B8A8_SRGB, 
		VK_IMAGE_LAYOUT_UNDEFINED, 
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		commandPool,
		graphicsFamily,
		device);
	
	Image::copyBufferToImage(
		stagingBuffer, 
		mTextureImage, 
		static_cast<SPX_INT>(texWidth), 
		static_cast<SPX_INT>(texHeight),
		commandPool,
		graphicsFamily,
		device);
	
	Image::transitionImageLayout(
		mTextureImage, 
		VK_FORMAT_R8G8B8A8_SRGB, 
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		commandPool,
		graphicsFamily,
		device);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void Texture::createTextureImageView(VkDevice device)
{
	mTextureImageView = ImageView::createImageView(mTextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, device);
}

void Texture::createTextureSampler(VkPhysicalDevice physicalDevice, VkDevice device)
{
	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(physicalDevice, &properties);

	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	// Specifies how to interpolate texels that are magnified or minimized.
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	// Addressing mode can be specified per axis. Available values:
	// VK_SAMPLER_ADDRESS_MODE_REPEAT: Repeat the texture when going beyond the image dimensions
	// VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT: Like repeat, but inverts the coordinates to mirror the image when going beyond
	// the image dimensions
	// VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE: Take the color edge closest to the coordinate beyond the image dimensions
	// VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE: Like clamp to edge, but instead uses the edge opposite to the closet edge
	// VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BOARDER: Return a solid color when sampling beyond the dimensions of the image.
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	// Specifies if anisotropic filtering should be used. No reason NOT to use this unless preformance is concern.
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
	// Specifies which color is returned when sampling beyond the image.
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	// Specifies which coordinate system I want to use to address texels in an image.
	// Bottom of page 218 explains other options
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	// If enabled then texels will first be compared to a value, and the result of that comparison is used in filtering operations.
	// Mainly used for percentage-closer filtering on shadow maps.
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

	if (vkCreateSampler(device, &samplerInfo, nullptr, &mTextureSampler) != VK_SUCCESS)
		CORE_ERROR("Failed to create a texture sampler.");
	else
		CORE_INFO("Texture Sampler created successfully.");
}
