#pragma once

#include <vulkan/vulkan.h>

namespace Murloc {

	class VulkanIndexBuffer {
	public:
		VulkanIndexBuffer(const uint32_t* indices, uint32_t count);
		~VulkanIndexBuffer();

		VkBuffer GetNative() const { return m_IndexBuffer; }
	private:
		uint32_t m_Count{ 0 };
		VkBuffer m_IndexBuffer{ VK_NULL_HANDLE };
		VkDeviceMemory m_IndexBufferMemory{ VK_NULL_HANDLE };
	};

}