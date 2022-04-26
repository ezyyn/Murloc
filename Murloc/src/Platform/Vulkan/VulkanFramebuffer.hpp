#pragma once

#include "VulkanRenderPass.hpp"
#include "VulkanSwapchain.hpp"

#include <vulkan/vulkan.h>

namespace Murloc {

	class VulkanFramebuffer {
	public:
		VulkanFramebuffer(const Ref<VulkanRenderPass>& renderpass,
			const Ref<VulkanSwapchain>& swapchain);
						  
		~VulkanFramebuffer();

		void RecreateFramebuffers(const Ref<VulkanRenderPass>& renderpass,
			const Ref<VulkanSwapchain>& swapchain);

		const std::vector<VkFramebuffer>& GetSwapchainFramebuffers() const { return m_SwapchainFramebuffers; }
		void Cleanup();
	private:
		std::vector<VkFramebuffer> m_SwapchainFramebuffers;
	};

}
