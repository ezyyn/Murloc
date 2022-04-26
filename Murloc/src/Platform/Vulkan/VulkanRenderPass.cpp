#include "murpch.hpp"

#include "VulkanRenderPass.hpp"
#include "Vulkan.hpp"

namespace Murloc {

	VulkanRenderPass::VulkanRenderPass(const Ref<VulkanSwapchain>& swapchain)
		: m_Swapchain(swapchain)
	{
		Recreate();
	}

	VulkanRenderPass::~VulkanRenderPass()
	{
		Cleanup();
	}

	void VulkanRenderPass::Begin(VkFramebuffer currentFb, VkCommandBuffer buffer)
	{
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_RenderPass;
		renderPassInfo.framebuffer = currentFb;
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = m_Swapchain->GetSwapChainExtent();

		VkClearValue clearColor = { {{0.2f, 0.2f, 0.2f, 1.0f}} };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(buffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	}

	void VulkanRenderPass::End(VkCommandBuffer buffer)
	{
		vkCmdEndRenderPass(buffer);
	}

	void VulkanRenderPass::Recreate()
	{
		// Attachment description
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = m_Swapchain->GetSwapchainImageFormat();
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		// Subpasses and attachment references
		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		// Render pass 
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		MUR_VK_ASSERT(vkCreateRenderPass(Vulkan::GetDevice()->GetNative(), &renderPassInfo, nullptr, &m_RenderPass));
	}

	void VulkanRenderPass::Cleanup()
	{
		vkDestroyRenderPass(Vulkan::GetDevice()->GetNative(), m_RenderPass, nullptr);
	}

}