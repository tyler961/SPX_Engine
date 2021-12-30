#include "VGraphicsPipeline.h"
#include "VShader.h"
#include "VDevice.h"


VGraphicsPipeline::VGraphicsPipeline(std::string vertFile, std::string fragFile, VDevice& device)
	:mDevice(device) {
	mShaders.push_back(VShader(ShaderType::VERT_SHADER, vertFile, mDevice));
	mShaders.push_back(VShader(ShaderType::FRAG_SHADER, fragFile, mDevice));
}

VGraphicsPipeline::~VGraphicsPipeline() {
	vkDestroyPipelineLayout(mDevice.mLogicalDevice, mPipelineLayout, nullptr);
	vkDestroyPipeline(mDevice.mLogicalDevice, mGraphicsPipeline, nullptr);
}

void VGraphicsPipeline::createGraphicsPipeline(VkExtent2D extent, VkDescriptorSetLayout descriptorSetLayout, VkRenderPass renderPass) {
	// **********************************************************************************************************************
	// SHADER
	// **********************************************************************************************************************
	VkPipelineShaderStageCreateInfo vertShaderInfo{};
	vertShaderInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;

	// Specify the shader module and which function to set as the entry point.
	// IMPORTANT: This lets me write multiple shaders in one file and change the functions to get different results.
	vertShaderInfo.module = mShaders.at(VERT).mShaderModule;
	vertShaderInfo.pName = "main";
	// OPTIONAL MEMBER: pSpecializationInfo. Allows me to specify values for shader constants. LOOK INTO LATER


	VkPipelineShaderStageCreateInfo fragShaderInfo{};
	fragShaderInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderInfo.module = mShaders.at(FRAG).mShaderModule;
	fragShaderInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderInfo, fragShaderInfo };



	// **********************************************************************************************************************
	// VERTEX INPUT
	// **********************************************************************************************************************
	// VkPipelineVertexInputStateCreateinfo struct describes the format of the vertex data that will be passed to the vertex shader.
	// Described in roughly two ways:
	// Bindings: Spacing between data and whether the data is per-vertex shader or per-instance
	// Attribute Descriptions: Type of the attributes passed to the vertex shader, which binding to load them from and at which offset
	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	auto bindingDescript = Vertex::getBindingDescription();
	auto attributeDescript = Vertex::getAttributeDescriptions();
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescript;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescript.size());
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescript.data();



	// **********************************************************************************************************************
	// INPUT ASSEMBLY
	// **********************************************************************************************************************
	// This describes what kind of geometry will be drawn from the vertices and if primitive restart should be enabled.
	// The former is specified in the topology member and can have values:
	// VK_PRIMITIVE_TOPOLOGY_POINT_LIST: Points from vertices
	// VK_PRIMITEVE TOPOLOGY_LINE_LIST: Line from every 2 vertices without reuse
	// VK_PRIMITEVE TOPOLOGY_LINE_STRIP: The end vertex of every line is used as a start vertex for the next line
	// VK_PRIMITEVE TOPOLOGY_TRIANGLE_LIST: Triangle from every 3 vertices without reuse
	// VK_PRIMITEVE TOPOLOGY_TRIANGLE_STRIP: The second and third vertex of every triangle are used as the first two vertices of the next triangle
	// With an element buffer I can sepcify the indices to use myself. This allows me to preform optimizations like reusing vertices
	// If I setup primitiveRestartEnable to VK_TRUE, this I can break up lines and triangles in the _STRIP topology by
	// using a special index of 0xFFFF or 0xFFFFFFFF.

	// For now I intend to draw triangles throughout, so I'll use the following
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;



	// **********************************************************************************************************************
	// VIEWPORT
	// ********************************************************************************************************************** 
	// Now setup the viewport
	// This setting will almost always be this

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)extent.width;
	viewport.height = (float)extent.height;
	// specifies the range of depth values to use for framebuffer. Typically stick to what's below
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	// Scissor rectangles define in which regions pixles will actually be stored.
	// Any outside the rectangle will be discarded.
	// Like a filter, instead of a viewport. Pg 108 has an illustration
	// For now I want to draw the entire framebuffer, so the scissor rectangle will cover the entire framebuffer
	VkRect2D scissor = {};
	scissor.offset = { 0,0 };
	scissor.extent = extent;

	// Using the view port and scissor, they need to be combined into a viewport state
	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;



	// **********************************************************************************************************************
	// RASTERIZER
	// **********************************************************************************************************************
	// Rasterizer
	// It takes the geometry that is shaped by the vertices from the vertex shader and turns it into fragments to be colored.
	// it also preforms depth testing, face culling and the scissor test, and can be configured to output fragments that fill
	// the entire polygons or just the edges (WIREFRAME RENDERING).
	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	// If this is set to true then fragments that are beyond the near and far planes are clamped to them
	// as opposed to discardng them. This is usefule in some special cases like shadow maps.
	rasterizer.depthClampEnable = VK_FALSE;
	// If this is set to true, then geometry never passes through the rasterizer stage. Bascially disables any output from the framebuffer
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	// Determines how fragments are generated for geometry. Following are available
	// VK_POLYGON_MODE_FILL: Fill the area of the polygon with fragments
	// VK_POLYGON_MODE_LINE: Polygon edges are drawn as lines
	// VK_POLYGON_MODE_POINT: Polygon vertices are drawn as points
	// Using any other mode than fill requires enabling a GPU feature
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	// Linewidth describes the thickness of lines in terms of number of fragments. Max that is supported is dependant
	// on the hardware and any line thicker than 1.0f requires me to enable the wideLines GPU feature
	rasterizer.lineWidth = 1.0f;
	// Cullmode variable defines the type of face culling to use. I can disable face culling, cull the font faces, 
	// cull the back faces or both. The frontFace variable specifies the vertex orer for faces to be considered front-facing
	// and can be clockwise or counterclockwise
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	// The rasterizer can alter the depth values by adding a constant value or biasing them based on fragements slope.
	// This is sometimes used for shadow mapping, but I won't be using it here
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f; // Optional
	rasterizer.depthBiasClamp = 0.0f;
	rasterizer.depthBiasSlopeFactor = 0.0f;



	// **********************************************************************************************************************
	// MULTISAMPLING
	// **********************************************************************************************************************
	// This struct configures the multisampling, which is one of the ways to preform anti-aliasing. It works by combining
	// the fragment shader results of multiple polygons that rasterize to the same pixel. This mainly occurs along edges, which
	// is alos where the most noticeable aliasing artifacts occur. Enabling this requires enabling a GPU feature.
	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f; // Optional
	multisampling.pSampleMask = nullptr; // Optional
	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampling.alphaToOneEnable = VK_FALSE; // Optional



	// **********************************************************************************************************************
	// DEPTH AND STENCIL TESTING
	// **********************************************************************************************************************
	// If I'm using depth and or stencil buffer, then I also need to configure the depth and stencil tests using
	// VkPipelineDepthStencilStateCreateInfo. I dont have one right now so we can simply pass a nullptr instead of pointer
	// to such a struct.
	VkPipelineDepthStencilStateCreateInfo depthStencil = {};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.0f; // Optional
	depthStencil.maxDepthBounds = 1.0f; // Optional
	depthStencil.stencilTestEnable = VK_FALSE;
	depthStencil.front = {}; // Optional
	depthStencil.back = {};



	// **********************************************************************************************************************
	// COLOR BLENDING
	// **********************************************************************************************************************
	// After a fragment shader has a color, it needs to be combined with the color that is already in the framebuffer.
	// This transformation is known as a color blending and there are two ways to do it:
	// Mix the old and new value to produce a final color
	// Combine the old and new value using a bitwise operation
	// There are tow different types of structs to configure color blending. The first struct, VkPipelineColorBlendAttachmentState
	// contains the configuration per attached framebuffer and the second struct, VkPipelineColorBlendStateCreateInfo contains
	// the global color blending settings. For thie I only have one framebuffer, // This per-frame buffer struct allows me to 
	// configure the first way of color blending. pg 111 for more info:
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
		VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
		VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f; // Optional
	colorBlending.blendConstants[1] = 0.0f; // Optional
	colorBlending.blendConstants[2] = 0.0f; // Optional
	colorBlending.blendConstants[3] = 0.0f; // Optional
	// For the second method of blending (bitwise) see page 112


	// **********************************************************************************************************************
	// DYNAMIC STATE
	// **********************************************************************************************************************
	// A limited amount of the state that I've specified in the previous structs can actually be changed without
	// recreating the pipeline. Examples are size of the viewport, line width and blend constants. If I do that, then I'll
	// have to fill in a VkPipelineDynamicStateCreateInfo like this:
	VkDynamicState dynamicStates[] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_LINE_WIDTH
	};

	VkPipelineDynamicStateCreateInfo dynamicState = {};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = 2;
	dynamicState.pDynamicStates = dynamicStates;



	// **********************************************************************************************************************
	// PIPELINE LAYOUT
	// **********************************************************************************************************************
	// I can use uniform values in shaders, which are globals similar to dynamic state variables that can be changed at
	// drawing time to alter the behavior of my shaders without having to recreate them. They are commonly used to pass
	// the transformation matrix to the vertex shader, or to create texture samplers in the fragment shader.
	// These uniform values need to be specified during pipeline creation by creating a VkPipelineLayout object.
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	// Checks if there is a descriptor set layout available. If not ignore these. Otherwise fill them in.
	if (descriptorSetLayout != VK_NULL_HANDLE) {
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout; // For now all my objects have the same descriptor set layouts.
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;
	}

	if (vkCreatePipelineLayout(mDevice.mLogicalDevice, &pipelineLayoutInfo, nullptr, &mPipelineLayout) != VK_SUCCESS)
		CORE_ERROR("Failed to create Pipeline Layout.");
	else
		CORE_INFO("Pipeline Layout created successfully.");



	// **********************************************************************************************************************
	// GRAPHICS PIPELINE
	// **********************************************************************************************************************
	// With everythind defined, time to create the graphics pipeline. NEED RENDER PASS ALSO
	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = nullptr; // Optional
	pipelineInfo.layout = mPipelineLayout;
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0; // INDEX OF SUBPASS
	// Vulkan allows me to create a new grpahics pipeline by deriving from an exisiting pipeline. The idea is that it is
	// less expensive than creating a brand new one. I can specify the handle of an exisiting pipeline or reference
	// LOOK ON PAGE 121 FOR HOW TO DO THAT, kinda how to it
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	pipelineInfo.basePipelineIndex = -1; // Optional

	// Takes more parameters than normal object creation in Vulkan. It is designed to take multiple VkGraphicsPipelineCreateInfo objects
	// and create multiple VkPipeline objects in a single call. 
	// Second parameter referecnes an optional VkPipelineCache object. A pipeline cache cam be used to store and reuse data
	// relevant to pipeline creation across multiple calls to vkCreateGraphicsPipelines and even across program
	// executions if the cache is stored to a file. This makes it possible to significalntly speed up pipeline creation
	// at a later time.
	if (vkCreateGraphicsPipelines(mDevice.mLogicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &mGraphicsPipeline) != VK_SUCCESS)
		CORE_ERROR("Failed to create Graphics Pipeline.");
	else
		CORE_INFO("Graphics Pipeline created successfully.");
}
