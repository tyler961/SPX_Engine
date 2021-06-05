#pragma once

#include "../../pch.h"
#include "Shader.h"
#include "Vertex.h"

// In the vector of shader classes, vert has to be in 0 and frag has to be in 1. 
// Will change later for more dynamic control
enum ShaderLocation { VERT, FRAG };

class GraphicsPipeline
{
public:
	GraphicsPipeline(std::string vertFile, std::string fragFile, VkDevice device);
	VkPipelineLayout mPipelineLayout;
	VkPipeline mGraphicsPipeline;

	std::vector<Shader> mShaders;

	void createGraphicsPipeline(
		VkDevice device, 
		VkExtent2D extent, 
		VkDescriptorSetLayout descriptorSetLayout,
		VkRenderPass renderPass);
};