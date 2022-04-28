#include "murpch.hpp"

#include "VulkanIndexBuffer.hpp"

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

	VulkanIndexBuffer::VulkanIndexBuffer(const uint32_t* indices, uint32_t count)
		: m_Count(count)
	{
		size_t size = sizeof(uint32_t) * count;
		auto device = Vulkan::GetDevice()->GetNative();

		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		MUR_VK_ASSERT(vkCreateBuffer(device, &bufferInfo, nullptr, &m_IndexBuffer));

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device, m_IndexBuffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = Utils::FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		MUR_VK_ASSERT(vkAllocateMemory(device, &allocInfo, nullptr, &m_IndexBufferMemory));

		vkBindBufferMemory(device, m_IndexBuffer, m_IndexBufferMemory, 0);

		void* data;
		vkMapMemory(device, m_IndexBufferMemory, 0, size, 0, &data);
		memcpy(data, indices, size);
		vkUnmapMemory(device, m_IndexBufferMemory);
	}

	VulkanIndexBuffer::~VulkanIndexBuffer()
	{
		auto device = Vulkan::GetDevice()->GetNative();

		vkDestroyBuffer(device, m_IndexBuffer, nullptr);
		vkFreeMemory(device, m_IndexBufferMemory, nullptr);
	}

}