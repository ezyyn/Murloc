#pragma once

#include <vulkan/vulkan.h>

namespace Murloc {

	class VulkanBuffer {
	public:
		VulkanBuffer(size_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
		~VulkanBuffer();

		void SetData(void* data);

		VkBuffer GetNative() const { return m_Buffer; }
	private:
		size_t m_Size;
		VkBuffer m_Buffer{ VK_NULL_HANDLE };
		VkDeviceMemory m_VertexBufferMemory{ VK_NULL_HANDLE };
	};

}