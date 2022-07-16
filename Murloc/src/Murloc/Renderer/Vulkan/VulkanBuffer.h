#pragma once

#include <vulkan/vulkan.h>

namespace Murloc {

	class VulkanBuffer {
	public:
		VulkanBuffer() = default;
		VulkanBuffer(size_t size, VkBufferUsageFlags bufferUsageFlags, VkMemoryPropertyFlags memoryProperties);
		VulkanBuffer(void* data, size_t size, VkBufferUsageFlags bufferUsageFlags, VkMemoryPropertyFlags memoryProperties);
		~VulkanBuffer();

		//operator VkBuffer() const { return m_Buffer; }

		VkBuffer GetBuffer() const { return m_Buffer; }

		size_t Size() const { return m_Size; }

		void SetData(const void* data);

	private:
		size_t m_Size{ 0 };
		VkBuffer m_Buffer{ VK_NULL_HANDLE };
		VkDeviceMemory m_Memory{ VK_NULL_HANDLE };
	};
}