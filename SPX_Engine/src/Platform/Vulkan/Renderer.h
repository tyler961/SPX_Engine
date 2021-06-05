#pragma once

#include "../../SPXENGINE/SPX_Engine/SPX_Engine/src/SPX/Core.h"
#include "../../pch.h"
#include "GLFW/glfw3.h"
#include "ValidationLayers.h"
#include "../../SPXENGINE/SPX_Engine/SPX_Engine/src/SPX/Window.h"
#include "Device.h"
#include "SwapChain.h"
#include "GraphicsPipeline.h"
#include "Image.h"
#include "CommandBuffer.h"
#include "Texture.h"

const int MAX_FRAMES_IN_FLIGHT = 2; // Used at the end of draw frame to limit work pile up from the cpu to the gpu

class Renderer
{
public:
	Renderer(Window* window);
	void createInstance();
	void setupDebug();
	void createSurface();
	void createRenderPass();
	void createDescriptorSetLayout(); // Move this later once I know exactly what it does
	void createCommandPool(); // Maybe move this to command buffer class later.
	void createDepthResources(); // Maybe move this somewhere else later.
	void loadModel(std::string modelPath); // MOVE THIS SO IT IS DYNAMIC
	void createVertexBuffer(); // Study this. Move this to an object class.
	void createIndexBuffer(); // Study this. Move this to an object class.
	void createUniformBuffers(); // Study this. Move this to an object class.
	void createDescriptorPool(); // Move this later once I know exactly what it does
	void createDescriptorSets(); // Move this later once I know exactly what it does
	void createSyncObjects(); // Move to sync classa
	void updateUniformBuffer(SPX_INT currentImage);
	VkFormat findDepthFormat();
	VkFormat findSupportedFormat(
		const std::vector<VkFormat>& candidates,
		VkImageTiling tiling, 
		VkFormatFeatureFlags features);

	ValidationLayers mValLayers;
	Device* mDevice;
	SwapChain* mSwapChain;
	GraphicsPipeline* mPipeline;
	CommandBuffer* mCommandBuffer;
	Texture* mTexture;

	VkRenderPass mRenderPass;
	VkDescriptorSetLayout mDescriptorSetLayout;
	VkDescriptorPool mDescriptorPool;
	std::vector<VkDescriptorSet> mDescriptorSets;
	VkCommandPool mCommandPool;

	// Move these somewhere else later.
	VkImage mDepthImage;
	VkImageView mDepthImageView;
	VkDeviceMemory mDepthImageMemory;
	// ...............................

	Window* mWindow;

	VkInstance mInstance; // Vulkan Instance
	VkSurfaceKHR mSurface; // Platform independent window object

	// Move later into an objects class
	std::vector<Vertex> vertices;
	std::vector<SPX_INT> indices;

	VkBuffer mVertexBuffer;
	VkDeviceMemory mVertexBufferMemory;

	VkBuffer mIndexBuffer;
	VkDeviceMemory mIndexBufferMemory;

	std::vector<VkBuffer> mUniformBuffers;
	std::vector<VkDeviceMemory> mUniformBuffersMemory;

	// Move to sync class
	std::vector<VkSemaphore> mImageAvailableSemaphores;
	std::vector<VkSemaphore> mRenderFinishedSemaphores;
	std::vector<VkFence> mInFlightFences;
	std::vector<VkFence> mImagesInFlight;

	size_t mCurrentFrame = 0;

	// Catches if the window is resized
	bool mFramedBufferResized = false;
};