#include "murpch.hpp"

#include "VulkanBuffer.hpp"

#include "Vulkan.hpp"

namespace Murloc {

	namespace Utils {

		static uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {

			VkPhysicalDeviceMemoryProperties memProperties;
			vkGetPhysicalDeviceMemoryProperties(Vulkan::GetDevice()->GetPhysicalDevice()->GetNative(), &memProperties);
			for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
				if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
					return i;
				}
			}
		}
	}

	VulkanBuffer::VulkanBuffer(size_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
		: m_Size(size)
	{
		auto device = Vulkan::GetDevice()->GetNative();

		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		MUR_VK_ASSERT(vkCreateBuffer(device, &bufferInfo, nullptr, &m_Buffer));

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device, m_Buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = Utils::FindMemoryType(memRequirements.memoryTypeBits, properties);

		MUR_VK_ASSERT(vkAllocateMemory(device, &allocInfo, nullptr, &m_VertexBufferMemory));

		vkBindBufferMemory(device, m_Buffer, m_VertexBufferMemory, 0);
	}

	VulkanBuffer::~VulkanBuffer()
	{
		auto device = Vulkan::GetDevice()->GetNative();

		vkDestroyBuffer(device, m_Buffer, nullptr);
		vkFreeMemory(device, m_VertexBufferMemory, nullptr);
	}

	void VulkanBuffer::SetData(void* source)
	{
		auto device = Vulkan::GetDevice()->GetNative();

		void* data;
		vkMapMemory(device, m_VertexBufferMemory, 0, (size_t)m_Size, 0, &data);
		memcpy(data, source, m_Size);
		vkUnmapMemory(device, m_VertexBufferMemory);
	}

}