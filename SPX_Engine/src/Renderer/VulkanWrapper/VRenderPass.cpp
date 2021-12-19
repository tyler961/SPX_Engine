#include "VRenderPass.h"
#include "VDevice.h"
#include "VSwapChain.h"
#include "VulkanHelperFunctions.h"

VRenderPass::VRenderPass(
    const VDevice& device, 
   // const std::vector<VkAttachmentDescription>& attachments,
   // const std::vector<VkSubpassDependency>& dependencies,
   // VkSubpassDescription subpassDescription, 
    const std::string& name,
	VSwapChain swapChain) // Remove swapchain variable once I sent the attachments and dependencies and subpass.
    : mDevice(device), mName(name)
{
	// This is to tell Vulkan about the framebuffer attachments that I will be using during rendering. I need
	// to specify how many color and depth buffers there will be, how many samples to use for each of them and how
	// their contents should be handled throughout the rendering operations. All of this info is wrapped in a render
	// pass object, for which I'll create a new createRenderPass function.
	// In this case I'll have just a single color buffer attachment represented by one of the images from the swap chain
	// This has a lot of settings that I will need to dynamically change once I actually begin an engine.
	VkAttachmentDescription colorAttach{};
	colorAttach.format = swapChain.mSwapChainImageFormat;
	// With no multisampling yet, setting it to one sample
	colorAttach.samples = VK_SAMPLE_COUNT_1_BIT;
	// Determines what to do with data in the attachment before rendering and after rendering.
	colorAttach.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // Clear the values to a constant at the start
	colorAttach.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // Rendered contents will be stored in memory and can be read later
	// No stencil yet so set to idc
	colorAttach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	// InitalLayout specifies which layout the image will have before render pass
	colorAttach.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	// FinalLayout specifies the layout to automatically transition to when the render pass finishes.
	// I want the image to be ready for presentation using the swapchain after rendering.
	colorAttach.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentDescription depthAttach{};
	depthAttach.format = VHF::VulkanHelperFunctions::findDepthFormat(mDevice.mPhysicalDevice);
	depthAttach.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttach.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttach.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttach.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttach.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	// pg 238
	VkAttachmentReference depthAttachRef = {};
	depthAttachRef.attachment = 1;
	depthAttachRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;


	// This array only has one description so index attachment set to 0 for the start of the array.
	VkAttachmentReference colorAttachRef{};
	colorAttachRef.attachment = 0;
	// Layout specifies which layout I would like the attachment to have during a subpass that uses this reference.
	colorAttachRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;


	// Subpass
	// Learn more about subasses.
	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; // Need to specifiy Graphics over Compute.
	// The index of the attachment in this array DIRECTLY REFERENCES the fragment shader with layout(location = 0) out vec4 outColor.
	// These other types of attachements can also be referenced.
	// pInputAttachments: Attachments that are read from a shader
	// pResolveAttachments: Attachments used for multisampling color attachments
	// pDepthStencilAttachment: Attachment for depth and stencil data
	// pPreserveAttachments: Attachments that are not used by this subpass, but for which the data must be preseved. 
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachRef;
	// Here is where depth attachment is hooked:
	subpass.pDepthStencilAttachment = &depthAttachRef;

	VkSubpassDependency depend{};
	depend.srcSubpass = VK_SUBPASS_EXTERNAL;
	depend.dstSubpass = 0;
	depend.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	depend.srcAccessMask = 0;
	depend.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	depend.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	// Now that the attachment and basic subpass referencing have been described, I can create the render pass.
	// The render pass object can be created by filling out the below structure with what I've created before.
	std::array<VkAttachmentDescription, 2> attachments = { colorAttach, depthAttach }; // Here I add depthAttach once I've done that.
	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &depend;

	if (vkCreateRenderPass(mDevice.mLogicalDevice, &renderPassInfo, nullptr, &mRenderPass) != VK_SUCCESS)
		CORE_ERROR("Failed to create a render pass.");
	else
		CORE_INFO("Render pass created successfully.");



    // USED ONCE I'M NO LONGER HARDCODING THE VARIABLES BEING PASSED HERE.
    /*VkRenderPassCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    createInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    createInfo.pAttachments = attachments.data();
    createInfo.subpassCount = 1;
    createInfo.pSubpasses = &subpassDescription;
    createInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
    createInfo.pDependencies = dependencies.data();

    CORE_TRACE("Creating renderpass {}.", name);

    if (vkCreateRenderPass(mDevice.mLogicalDevice, &createInfo, nullptr, &mRenderPass) != VK_SUCCESS)
        CORE_ERROR("Error: vkCreateRenderPass failed for {}.", mName);

    CORE_TRACE("Created renderpass successfully.");*/
}

VRenderPass::~VRenderPass()
{
	//vkDestroyRenderPass(mDevice.mLogicalDevice, mRenderPass, nullptr);
}



