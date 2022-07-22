#include "pgpch.h"

#include "IndexBuffer.h"

#include "VulkanContext.h"
#include "VulkanUtils.h"

namespace PG {

	IndexBuffer::IndexBuffer(const uint32_t* indices, uint32_t count)
		: m_Count(count)
	{
		auto device = VulkanContext::GetLogicalDevice()->GetNative();
		size_t size = sizeof(uint32_t) * count;

		// Index buffer
		// Index buffer
		// Index buffer
		{
			VkBufferCreateInfo bufferInfo{};
			bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferInfo.size = size;
			bufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			PG_VK_ASSERT(vkCreateBuffer(device, &bufferInfo, nullptr, &m_IndexBuffer));

			VkMemoryRequirements memRequirements;
			vkGetBufferMemoryRequirements(device, m_IndexBuffer, &memRequirements);

			VkMemoryAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = Utils::FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

			PG_VK_ASSERT(vkAllocateMemory(device, &allocInfo, nullptr, &m_IndexBufferMemory));

			vkBindBufferMemory(device, m_IndexBuffer, m_IndexBufferMemory, 0);
		}

		// Staging buffer
		// Staging buffer
		// Staging buffer
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		{
			VkBufferCreateInfo bufferInfo{};
			bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferInfo.size = size;
			bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			PG_VK_ASSERT(vkCreateBuffer(device, &bufferInfo, nullptr, &stagingBuffer));

			VkMemoryRequirements memRequirements;
			vkGetBufferMemoryRequirements(device, stagingBuffer, &memRequirements);

			VkMemoryAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = Utils::FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

			PG_VK_ASSERT(vkAllocateMemory(device, &allocInfo, nullptr, &stagingBufferMemory));

			vkBindBufferMemory(device, stagingBuffer, stagingBufferMemory, 0);
		}
		// Set data into the staging buffer
		// Set data into the staging buffer
		// Set data into the staging buffer
		void* data;
		vkMapMemory(device, stagingBufferMemory, 0, size, 0, &data);
		memcpy(data, indices, size);
		vkUnmapMemory(device, stagingBufferMemory);

		// Creates and executes transfer command buffer
		CopyData(stagingBuffer, m_IndexBuffer);

		// Destroy staging buffer
		// Destroy staging buffer
		// Destroy staging buffer
		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);

		// Swapchain resource free queue
		{
			auto& resourceFreeQueue = VulkanContext::GetContextResourceFreeQueue();

			resourceFreeQueue.PushBack(BUFFER, [m_IndexBuffer = m_IndexBuffer,
				m_IndexBufferMemory = m_IndexBufferMemory]()
				{
					auto device = VulkanContext::GetLogicalDevice()->GetNative();

					// Destroy vertex buffer
					vkDestroyBuffer(device, m_IndexBuffer, nullptr);
					vkFreeMemory(device, m_IndexBufferMemory, nullptr);
				});
		}
	}

	IndexBuffer::~IndexBuffer()
	{
	}

	void IndexBuffer::CopyData(VkBuffer src, VkBuffer dst)
	{
		auto device = VulkanContext::GetLogicalDevice();

		ScopeCommandPool tempPool; // For short-live cmd buffers

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = tempPool.GetNative();
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(device->GetNative(), &allocInfo, &commandBuffer);
		{
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			vkBeginCommandBuffer(commandBuffer, &beginInfo);

			VkBufferCopy copyRegion{};
			copyRegion.srcOffset = 0; // Optional
			copyRegion.dstOffset = 0; // Optional
			copyRegion.size = m_Count * sizeof(uint32_t);
			vkCmdCopyBuffer(commandBuffer, src, dst, 1, &copyRegion);

			vkEndCommandBuffer(commandBuffer);
		}

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(device->GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);

		vkQueueWaitIdle(device->GetGraphicsQueue());  // <- Should use fence

		vkFreeCommandBuffers(device->GetNative(), tempPool.GetNative(), 1, &commandBuffer);
	}

}