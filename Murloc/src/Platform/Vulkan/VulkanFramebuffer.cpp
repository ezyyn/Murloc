#include "murpch.hpp"

#include "VulkanFramebuffer.hpp"

#include "Vulkan.hpp"

namespace Murloc {

	VulkanFramebuffer::VulkanFramebuffer(const Ref<VulkanRenderPass>& renderpass,
		const Ref<VulkanSwapchain>& swapchain)
	{
		RecreateFramebuffers(renderpass, swapchain);
	}

	VulkanFramebuffer::~VulkanFramebuffer()
	{
		Cleanup();
	}

	void VulkanFramebuffer::RecreateFramebuffers(const Ref<VulkanRenderPass>& renderpass,
		const Ref<VulkanSwapchain>& swapchain)
	{
		m_SwapchainFramebuffers.resize(swapchain->GetImageViews().size());
		for (size_t i = 0; i < swapchain->GetImageViews().size(); i++) {

			VkImageView attachments[] = {
				swapchain->GetImageViews()[i]
			};

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderpass->GetNative();
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = swapchain->GetSwapChainExtent().width;
			framebufferInfo.height = swapchain->GetSwapChainExtent().height;
			framebufferInfo.layers = 1;

			MUR_VK_ASSERT(vkCreateFramebuffer(Vulkan::GetDevice()->GetNative(), &framebufferInfo, nullptr, &m_SwapchainFramebuffers[i]));
		}
	}

	void VulkanFramebuffer::Cleanup()
	{
		for (auto framebuffer : m_SwapchainFramebuffers) {
			vkDestroyFramebuffer(Vulkan::GetDevice()->GetNative(), framebuffer, nullptr);
		}
	}

}
