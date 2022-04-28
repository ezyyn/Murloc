#pragma once

#include <vulkan/vulkan.h>

#include "VulkanSwapchain.hpp"

namespace Murloc {

	class VulkanRenderPass {
	public:
		VulkanRenderPass();

		VkRenderPass GetNative() const { return m_RenderPass; }

		~VulkanRenderPass();

		void Begin(VkFramebuffer currentFb, VkCommandBuffer buffer);
		void End(VkCommandBuffer buffer);

		void CleanupAndRecreate(uint32_t width, uint32_t height);
	private:
		void Cleanup();

		VkExtent2D m_Extent;
		VkFormat m_SwapchainImageFormat;

		VkRenderPass m_RenderPass{ VK_NULL_HANDLE };
	};

}