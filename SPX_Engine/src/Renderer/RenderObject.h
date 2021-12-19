#pragma once

#include "../pch.h"

// For now this is just a struct to hold the mesh and texture data for each object to be drawn.
// Later it will be a component added to an actor to control it's rendering

class Mesh;
class Texture;
class VDevice;
class VCommandPool;

// TODO: Add a unique descriptor set for each RenderObject. Also need to have a cache of them to prevent duplicate
// descriptor sets for the exact same shaders. Same with Descriptor Set Layouts and MAYBE (CHECK THIS) Descriptor Pools.
// Definitely need descriptor sets for each since they are used for the textures.
// TODO: Seems a different PipelineLayout is also needed. Pass in the shader information and descriptor sets.

// Find out when I would have to update my descriptor sets.

class RenderObject
{
public:
	// Render Object is created just linking the file locations of the objects.
	RenderObject(std::string meshLoc, std::string textureLoc);
	~RenderObject();

	void drawObject(VkCommandBuffer cmd, VkPipelineLayout pipelineLayout, uint32_t currentImage);
	void updateUniformBuffers(VkExtent2D extent, uint32_t currentImage);

	// Loads buffers, textures and descriptors.
	void init(VCommandPool commandPool, uint32_t swapchainImages);

	// Loads the vertex and index information
	void loadBuffers();

	// Loads the objects descriptors
	void loadDescriptorInfo(uint32_t swapchainImages);
	void createDescriptorPool(uint32_t swapchainImages);
	void createDescriptorSetLayout(VDevice& device);
	void createDescriptorSets(uint32_t swapchainImages);

	// Change from pointers later.
	Mesh* mMesh;
	Texture* mTexture;

	VkDescriptorPool mDescriptorPool;
	VkDescriptorSetLayout mDescriptorSetLayout;
	std::vector<VkDescriptorSet> mDescriptorSets;

	VDevice* mDevice;

	glm::mat4 mTransformMatrix;

	std::string mMeshFileLocation;
	std::string mTextureFileLocation;
};