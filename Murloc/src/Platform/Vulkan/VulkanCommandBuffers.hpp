#pragma once

#include <vulkan/vulkan.h>

namespace Murloc {

	class VulkanCommandBuffers {
	public:
		VulkanCommandBuffers(int framesInFlight);
		~VulkanCommandBuffers();

		VkCommandBuffer GetNative(uint32_t index) const { return m_CommandBuffers[index]; };

		void Begin(uint32_t index);
		void End(uint32_t index);
	private:
		std::vector<VkCommandBuffer> m_CommandBuffers;

		VkCommandPool m_CommandPool;
	};

}