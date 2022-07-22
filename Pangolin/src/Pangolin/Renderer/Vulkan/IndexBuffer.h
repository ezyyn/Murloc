#pragma once

#include <vulkan/vulkan.h>

namespace PG {

	class IndexBuffer {
	public:
		IndexBuffer(const uint32_t* indices, uint32_t count);
		~IndexBuffer();

		VkBuffer GetNative() const { return m_IndexBuffer; }
	private:
		void CopyData(VkBuffer src, VkBuffer dst);

		uint32_t m_Count{ 0 };
		VkBuffer m_IndexBuffer{ VK_NULL_HANDLE };
		VkDeviceMemory m_IndexBufferMemory{ VK_NULL_HANDLE };
	};

}