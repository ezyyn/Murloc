#include "murpch.hpp"

#include "VulkanBuffer.h"

#include "VulkanContext.h"
#include "VulkanUtils.h"

namespace Murloc {

	VulkanBuffer::VulkanBuffer(size_t size, VkBufferUsageFlags bufferUsageFlags, VkMemoryPropertyFlags memoryProperties)
		: m_Size(size)
	{
		auto device = VulkanContext::GetLogicalDevice()->GetNative();

		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = bufferUsageFlags;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		MUR_VK_ASSERT(vkCreateBuffer(device, &bufferInfo, nullptr, &m_Buffer));

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device, m_Buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = Utils::FindMemoryType(memRequirements.memoryTypeBits, memoryProperties);

		MUR_VK_ASSERT(vkAllocateMemory(device, &allocInfo, nullptr, &m_Memory));

		MUR_VK_ASSERT(vkBindBufferMemory(device, m_Buffer, m_Memory, 0));

		auto& resourceFreeQueue = VulkanContext::GetContextResourceFreeQueue();

		resourceFreeQueue.PushBack(BUFFER, [device, m_Buffer = m_Buffer, m_Memory = m_Memory]()
			{
				vkDestroyBuffer(device, m_Buffer, nullptr);
				vkFreeMemory(device, m_Memory, nullptr);
			});
	}

	VulkanBuffer::VulkanBuffer(void* data, size_t size, VkBufferUsageFlags bufferUsageFlags, VkMemoryPropertyFlags memoryProperties)
		: VulkanBuffer(size, bufferUsageFlags, memoryProperties)
	{
		SetData(data);
	}

	VulkanBuffer::~VulkanBuffer()
	{
	}

	void VulkanBuffer::SetData(const void* data)
	{
		auto device = VulkanContext::GetLogicalDevice()->GetNative();

		void* dest;
		MUR_VK_ASSERT(vkMapMemory(device, m_Memory, 0, m_Size, 0, &dest));
		memcpy(dest, data, m_Size);
		vkUnmapMemory(device, m_Memory);
	}

}