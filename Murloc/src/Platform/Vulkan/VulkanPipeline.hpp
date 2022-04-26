#pragma once

#include "VulkanShader.hpp"
#include "VulkanSwapchain.hpp"
#include "VulkanRenderPass.hpp"

#include <vulkan/vulkan.h>

#include "Murloc/Renderer/BufferLayout.hpp"

namespace Murloc {

	class VulkanPipeline {
	public:
		VulkanPipeline(	const Ref<VulkanShader>& shader, 
						const Ref<VulkanRenderPass>& renderpass, 
						const Ref<VulkanSwapchain>& swapchain,
						const BufferLayout& layout);
		~VulkanPipeline();

		VkPipeline GetNative() const { return m_GraphicsPipeline; }

		void Recreate(const Ref<VulkanRenderPass>& renderpass, const Ref<VulkanSwapchain>& swapchain);
		void Cleanup();
		void Bind(VkCommandBuffer commandBuffer);
	private:
		Ref<VulkanShader> m_Shader;

		BufferLayout m_BufferLayout;

		VkPipeline m_GraphicsPipeline{ VK_NULL_HANDLE };
		VkPipelineLayout m_PipelineLayout{ VK_NULL_HANDLE };
	};

}