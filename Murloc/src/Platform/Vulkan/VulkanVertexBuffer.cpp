#include "murpch.hpp"

#include "VulkanVertexBuffer.hpp"

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

	VulkanVertexBuffer::VulkanVertexBuffer(size_t size)
		: m_Size(size)
	{
		auto device = Vulkan::GetDevice()->GetNative();

	/*	VkBuffer stagingBuffer; // Not sure if this is useful
		VkDeviceMemory stagingBufferMemory;
		// Staging buffer
		{
			VkBufferCreateInfo bufferInfo{};
			bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferInfo.size = size;
			bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			MUR_VK_ASSERT(vkCreateBuffer(device, &bufferInfo, nullptr, &stagingBuffer));

			VkMemoryRequirements memRequirements;
			vkGetBufferMemoryRequirements(device, stagingBuffer, &memRequirements);

			VkMemoryAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = Utils::FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

			MUR_VK_ASSERT(vkAllocateMemory(device, &allocInfo, nullptr, &stagingBufferMemory));

			vkBindBufferMemory(device, stagingBuffer, stagingBufferMemory, 0);
		}*/

		// Destination vertex buffer
		{
			VkBufferCreateInfo bufferInfo{};
			bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferInfo.size = size;
			bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
			bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			MUR_VK_ASSERT(vkCreateBuffer(device, &bufferInfo, nullptr, &m_VertexBuffer));

			VkMemoryRequirements memRequirements;
			vkGetBufferMemoryRequirements(device, m_VertexBuffer, &memRequirements);

			VkMemoryAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = Utils::FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

			MUR_VK_ASSERT(vkAllocateMemory(device, &allocInfo, nullptr, &m_VertexBufferMemory));

			vkBindBufferMemory(device, m_VertexBuffer, m_VertexBufferMemory, 0);
		}
	}

	VulkanVertexBuffer::~VulkanVertexBuffer()
	{
		auto device = Vulkan::GetDevice()->GetNative();

		vkDestroyBuffer(device, m_VertexBuffer, nullptr);
		vkFreeMemory(device, m_VertexBufferMemory, nullptr);
	}

	void VulkanVertexBuffer::SetData(void* source)
	{
		auto device = Vulkan::GetDevice()->GetNative();

		void* data;
		vkMapMemory(device, m_VertexBufferMemory, 0, (size_t)m_Size, 0, &data);
		memcpy(data, source, m_Size);
		vkUnmapMemory(device, m_VertexBufferMemory);
	}

}