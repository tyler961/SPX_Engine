#pragma once

#include "../../pch.h"
#include "DataStructures.h"

class VDevice;
class VShader;

enum ShaderLocation { VERT, FRAG };

// Depending on how I do things, the renderer won't have one graphics pipeline. It could be attached to models, IDK yet.

class VGraphicsPipeline {
public:
	VGraphicsPipeline(std::string vertFile, std::string fragFile, VDevice& device);
	~VGraphicsPipeline();

	void createGraphicsPipeline(VkExtent2D extent,
		VkDescriptorSetLayout descriptorSetLayout,
		VkRenderPass renderPass);

	// Might have to move this to somewhere else
	VkPipelineLayout mPipelineLayout;
	VkPipeline mGraphicsPipeline;

private:
	VDevice& mDevice;
	std::vector<VShader> mShaders;
};