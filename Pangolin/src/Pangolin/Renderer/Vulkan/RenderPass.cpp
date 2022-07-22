#include "pgpch.h"

#include "RenderPass.h"

#include "VulkanContext.h"

namespace PG {

	RenderPass::RenderPass(const RenderPassInfo& info)
		: m_RenderPassInfo(info)
	{
		auto device = VulkanContext::GetLogicalDevice()->GetNative();
		auto& physicalDeviceSupp = VulkanContext::GetLogicalDevice()->GetPhysicalDevice().GetSupportDetails();

		// Attachment description
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = physicalDeviceSupp.SurfaceFormat.format;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		// Subpasses and attachment references
		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpassDescription = {};
		subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDescription.colorAttachmentCount = 1;
		subpassDescription.pColorAttachments = &colorAttachmentRef;
		subpassDescription.inputAttachmentCount = 0;
		subpassDescription.pInputAttachments = nullptr;
		subpassDescription.preserveAttachmentCount = 0;
		subpassDescription.pPreserveAttachments = nullptr;
		subpassDescription.pResolveAttachments = nullptr;

		// Render pass 
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpassDescription;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;
		renderPassInfo.pSubpasses;

		PG_VK_ASSERT(vkCreateRenderPass(device,
			&renderPassInfo, nullptr, &m_RenderPass));

		VulkanContext::GetContextResourceFreeQueue().PushBack(RENDERPASS, [device, &renderPass = m_RenderPass]()
			{
				vkDestroyRenderPass(device, renderPass, nullptr);
			});
	}

	RenderPass::RenderPass(const VkRenderPassCreateInfo& info)
	{
		auto device = VulkanContext::GetLogicalDevice()->GetNative();

		PG_VK_ASSERT(vkCreateRenderPass(device,
			&info, nullptr, &m_RenderPass));

		VulkanContext::GetContextResourceFreeQueue().PushBack(RENDERPASS, [device, &renderPass = m_RenderPass]()
			{
				vkDestroyRenderPass(device, renderPass, nullptr);
			});
	}

	RenderPass::RenderPass(VkRenderPass renderPass)
		: m_RenderPass(renderPass)
	{
		auto device = VulkanContext::GetLogicalDevice()->GetNative();

		VulkanContext::GetContextResourceFreeQueue().PushBack(RENDERPASS, [device, &renderPass = m_RenderPass]()
			{
				vkDestroyRenderPass(device, renderPass, nullptr);
			});
	}

	RenderPass::~RenderPass()
	{
	}

}