#pragma once

#include <vulkan/vulkan.h>

namespace Murloc {

	class VertexBuffer {
	public:
		VertexBuffer(size_t size);
		~VertexBuffer();

		void SetData(void* data, uint32_t size);

		VkBuffer GetNative() const { return m_VertexBuffer; }
	private:
		void CopyData(VkBuffer src, VkBuffer dst, size_t size);

		VkBuffer m_StagingBuffer{ VK_NULL_HANDLE };
		VkDeviceMemory m_StagingBufferMemory{ VK_NULL_HANDLE };

		size_t m_Size;
		VkBuffer m_VertexBuffer{ VK_NULL_HANDLE };
		VkDeviceMemory m_VertexBufferMemory{ VK_NULL_HANDLE };
	};

}