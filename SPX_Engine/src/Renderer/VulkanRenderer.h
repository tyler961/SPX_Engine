#pragma once

// This class will have all of the wrapper class and create it all based on the information sent to it by the engine class
// Basically the interface between the engine and vulkan. The engine should never have to interact with anything other than the renderer
// for Vulkan.

// Renderer will also handle all the draw commands via classes the Engine sends to it, such as a model, light, etc.

// TODO: For now the semaphores and fences are hardcoded into the Renderer. Later change this to better allow for multithreading.

#include "../pch.h"
#include "VulkanWrapper/DataStructures.h"

const int MAX_FRAMES_IN_FLIGHT = 2; // Used at the end of draw frame to limit work pile up from the cpu to the gpu

class Window;
class VCommandPool;
class VInstance;
class VDevice;
class VSurface;
class VSwapChain;
class RenderObject;
class VRenderPass;
class VGraphicsPipeline;


class VulkanRenderer {
public:
	VulkanRenderer(Window* window);
	~VulkanRenderer();

	void init(std::string appName, std::string engineName, bool enableValLayers);

	void addRenderObject(RenderObject& renderObj);
	void addRenderObjects(std::vector<RenderObject> renderObjs);

	void draw(glm::mat4 cameraViewMatrix);

	// Run VMA memory stats
	void calculateMemoryBudget();
	void createSyncObjects();
	void createCommandBuffers();
	// Loads the Mesh and Texture data from the RenderObject to the GPU.
	void loadRenderObjects();

	bool mWindowResized{ false };
	bool mTimePassed{ 0.0f };
	// Timestep mStopwatch?

	std::vector<VkCommandBuffer> mMainCommandBuffers;

	uint32_t mCurrentFrame{ 0 };
	bool mFrameBufferResized{ false };

private:
	// Camera class
	// glfwContext
	Window* mWindow{ nullptr };
	VInstance* mInstance{ nullptr };
	VDevice* mDevice{ nullptr };
	VSurface* mSurface{ nullptr };
	VSwapChain* mSwapChain{ nullptr };
	VRenderPass* mRenderPass{ nullptr };
	VCommandPool* mCommandPool{ nullptr };
	VGraphicsPipeline* mGraphicsPipeline{ nullptr };
	std::vector<RenderObject> mRenderObjects;
	// TODO: imGUI overlay


	// Move to sync class
	std::vector<VkSemaphore> mImageAvailableSemaphores;
	std::vector<VkSemaphore> mRenderFinishedSemaphores;
	std::vector<VkFence> mInFlightFences;
	std::vector<VkFence> mImagesInFlight;


	// Move these to a descriptor class
	VkDescriptorSetLayout mDescriptorSetLayout;
	VkDescriptorPool mDescriptorPool;
	std::vector<VkDescriptorSet> mDescriptorSets;

	// RenderGraph?
};