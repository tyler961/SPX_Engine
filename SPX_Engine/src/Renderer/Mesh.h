#pragma once

#include "../pch.h"
#include "VulkanWrapper/DataStructures.h"
#include "../ThirdParty/vk_mem_alloc.h"

// Will add more to this as I develop the need for more information about the mesh.


class VDevice;



class Mesh
{
public:
	Mesh(std::string fileLocation, VDevice& device, size_t numSwapChainImages);
	~Mesh();


	void createBuffers();
	void createVertexBuffer();
	void createIndexBuffer();
	// TODO: Figure out if I really need a uniform buffer for each swapchain image. So far as I understand
	//		 it removes the possiblity of trying to update a buffer while it is being accessed.
	void createUniformBuffers();
	void updateUniformBuffers(VkExtent2D extent, uint32_t currentImage);

	std::vector<Vertex> mVertices;
	std::vector<uint32_t> mIndices;

	AllocatedBuffer mVertexBuffer;
	AllocatedBuffer mIndexBuffer;
	std::vector<AllocatedBuffer> mUniformBuffers;


private:
	VDevice& mDevice;
	size_t mNumSwapChainImages{ 0 };
};