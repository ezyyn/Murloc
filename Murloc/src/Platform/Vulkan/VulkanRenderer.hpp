#pragma once

#include "VulkanVertexBuffer.hpp"
#include "VulkanIndexBuffer.hpp"

#include "VulkanSwapchain.hpp"

namespace Murloc {

	class VulkanRenderer {
	public:
		static void Init();
		static void Shutdown();

		static void RecreateGraphicsPipeline();

		static VkRenderPass GetRenderPass();

		static VkCommandBuffer GetCurrentDrawCommandBuffer();

		static const Ref<VulkanSwapchain>& GetSwapchain();

		static uint32_t FramesInFlight() { return 2; } // TEMPORARY

		static void WaitIdle();

		static void DrawIndexed(const Ref<VulkanVertexBuffer>& vertexBuffer, const Ref<VulkanIndexBuffer>& indexBuffer, uint32_t count);
	private:

	};
}