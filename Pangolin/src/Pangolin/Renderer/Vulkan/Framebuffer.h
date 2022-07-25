#pragma once

#include <vulkan/vulkan.h>

#include "RenderPass.h"

namespace PG {

	struct FramebufferInfo {
		Ref<RenderPass> Renderpass;

		uint32_t Width;
		uint32_t Height;
		VkImageView ColorAttachment;
		VkImageView DepthAttachment;
	};

	class Framebuffer
	{
	public:
		Framebuffer(const FramebufferInfo& info);
		~Framebuffer() {}

		VkFramebuffer GetNative() const { return m_Framebuffer; }

		void Invalidate(uint32_t width, uint32_t height, VkImageView colorAttachment, VkImageView depthAttachment);
	private:
		FramebufferInfo m_Info;
		VkFramebuffer m_Framebuffer{ VK_NULL_HANDLE };
	};

}