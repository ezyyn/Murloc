#pragma once

#include <vulkan/vulkan.h>

#include "VulkanSwapchain.hpp"

namespace Murloc {

	class VulkanRenderPass {
	public:
		VulkanRenderPass(const Ref<VulkanSwapchain>& swapchain);

		VkRenderPass GetNative() const { return m_RenderPass; }

		~VulkanRenderPass();

		void Begin(VkFramebuffer currentFb, VkCommandBuffer buffer);
		void End(VkCommandBuffer buffer);

		void Recreate();
		void Cleanup();
	private:
		Ref<VulkanSwapchain> m_Swapchain;
		VkRenderPass m_RenderPass{ VK_NULL_HANDLE };
	};

}