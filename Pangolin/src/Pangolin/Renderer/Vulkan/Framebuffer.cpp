#include "pgpch.h"
#include "Framebuffer.h"

#include "VulkanContext.h"

namespace PG {

	Framebuffer::Framebuffer(const FramebufferInfo& info)
		: m_Info(info)
	{
		Invalidate(info.Width, info.Height, info.ColorAttachment, info.DepthAttachment);

		VulkanContext::GetContextResourceFreeQueue().PushBack(FRAMEBUFFER, [m_Framebuffer = &m_Framebuffer]()
			{
				auto device = VulkanContext::GetLogicalDevice()->GetNative();
				vkDestroyFramebuffer(device, *m_Framebuffer, nullptr);
			});
	}

	void Framebuffer::Invalidate(uint32_t width, uint32_t height, VkImageView colorAttachment, VkImageView depthAttachment)
	{
		m_Info.Width = width;
		m_Info.Height = height;
		m_Info.ColorAttachment = colorAttachment;
		m_Info.DepthAttachment = depthAttachment;
		
		auto device = VulkanContext::GetLogicalDevice()->GetNative();

		if (m_Framebuffer) {
			vkDestroyFramebuffer(device, m_Framebuffer, nullptr);
			m_Framebuffer = VK_NULL_HANDLE;
		}

		VkImageView attachments[]{
			m_Info.ColorAttachment,
			m_Info.DepthAttachment
		};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = m_Info.Renderpass->GetNative();
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.attachmentCount = 2; 
		framebufferInfo.width = m_Info.Width;
		framebufferInfo.height = m_Info.Height;
		framebufferInfo.layers = 1;

		PG_VK_ASSERT(vkCreateFramebuffer(VulkanContext::GetLogicalDevice()->GetNative(),
			&framebufferInfo, nullptr, &m_Framebuffer));
	}
}

