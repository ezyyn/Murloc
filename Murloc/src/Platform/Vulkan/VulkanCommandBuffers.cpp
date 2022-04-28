#include "murpch.hpp"

#include "VulkanCommandBuffers.hpp"
#include "Vulkan.hpp"
#include "VulkanRenderer.hpp"

namespace Murloc {

	VulkanCommandBuffers::VulkanCommandBuffers()
	{
		// Command pool
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = Vulkan::GetDevice()->GetPhysicalDevice()->GetQueueFamilyIndices().GraphicsFamily.value();

		MUR_VK_ASSERT(vkCreateCommandPool(Vulkan::GetDevice()->GetNative(), &poolInfo, nullptr, &m_CommandPool));

		// Command Buffers
		m_CommandBuffers.resize(VulkanRenderer::FramesInFlight());

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = m_CommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)m_CommandBuffers.size();

		MUR_VK_ASSERT(vkAllocateCommandBuffers(Vulkan::GetDevice()->GetNative(), &allocInfo, m_CommandBuffers.data()));
	}
	VulkanCommandBuffers::~VulkanCommandBuffers()
	{
		vkDestroyCommandPool(Vulkan::GetDevice()->GetNative(), m_CommandPool, nullptr);
	}

	void VulkanCommandBuffers::Begin(uint32_t index)
	{
		vkResetCommandBuffer(GetNative(index), 0);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0; // Optional
		beginInfo.pInheritanceInfo = nullptr; // Optional

		MUR_VK_ASSERT(vkBeginCommandBuffer(m_CommandBuffers[index], &beginInfo));
	}

	void VulkanCommandBuffers::End(uint32_t index)
	{
		MUR_VK_ASSERT(vkEndCommandBuffer(m_CommandBuffers[index]));
	}

}