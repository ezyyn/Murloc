#pragma once

#include <vulkan/vulkan.h>

namespace PG {

	class VertexBuffer {
	public:
		VertexBuffer(size_t size);
		~VertexBuffer();

		void SetData(void* data, uint32_t size, uint32_t currentFrame);

		VkBuffer GetNative() const { return m_VertexBuffer; }

		const std::vector<VkCommandBuffer>& GetCommandBuffer() const { return m_CopyDataCmdBuffers; }

	private:
		void CopyData(VkBuffer src, VkBuffer dst, size_t size, uint32_t currentFrame);

		std::vector<VkCommandBuffer> m_CopyDataCmdBuffers{ VK_NULL_HANDLE };;
		
		VkBuffer m_StagingBuffer{ VK_NULL_HANDLE };
		VkDeviceMemory m_StagingBufferMemory{ VK_NULL_HANDLE };
		void* m_StagingBufferMapPtr{ nullptr };

		size_t m_Size;
		VkBuffer m_VertexBuffer{ VK_NULL_HANDLE };
		VkDeviceMemory m_VertexBufferMemory{ VK_NULL_HANDLE };
	};

}