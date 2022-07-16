#include "murpch.hpp"

#include "VertexBuffer.h"

#include "VulkanContext.h"
#include "VulkanUtils.h"

namespace Murloc {

	VertexBuffer::VertexBuffer(size_t size)
		: m_Size(size)
	{
		auto device = VulkanContext::GetLogicalDevice()->GetNative();

		// Vertex buffer
		// Vertex buffer
		// Vertex buffer
		{
			VkBufferCreateInfo bufferInfo{};
			bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferInfo.size = size;
			bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			MUR_VK_ASSERT(vkCreateBuffer(device, &bufferInfo, nullptr, &m_VertexBuffer));

			VkMemoryRequirements memRequirements;
			vkGetBufferMemoryRequirements(device, m_VertexBuffer, &memRequirements);

			VkMemoryAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = memRequirements.size;
			// VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT - Memory allocated with this type can be mapped for host access using vkMapMemory
			allocInfo.memoryTypeIndex = Utils::FindMemoryType(memRequirements.memoryTypeBits,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT); // VRAM ?

			MUR_VK_ASSERT(vkAllocateMemory(device, &allocInfo, nullptr, &m_VertexBufferMemory));

			vkBindBufferMemory(device, m_VertexBuffer, m_VertexBufferMemory, 0);
		}

		// Staging buffer
		// Staging buffer
		// Staging buffer
		{
			VkBufferCreateInfo bufferInfo{};
			bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferInfo.size = size;
			bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			MUR_VK_ASSERT(vkCreateBuffer(device, &bufferInfo, nullptr, &m_StagingBuffer));

			VkMemoryRequirements memRequirements;
			vkGetBufferMemoryRequirements(device, m_StagingBuffer, &memRequirements);

			VkMemoryAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = Utils::FindMemoryType(memRequirements.memoryTypeBits,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

			MUR_VK_ASSERT(vkAllocateMemory(device, &allocInfo, nullptr, &m_StagingBufferMemory));

			vkBindBufferMemory(device, m_StagingBuffer, m_StagingBufferMemory, 0);
		}

		// Swapchain resource free queue
		{
			auto& resourceFreeQueue = VulkanContext::GetContextResourceFreeQueue();

			resourceFreeQueue.PushBack(BUFFER, [m_StagingBuffer = m_StagingBuffer,
				m_StagingBufferMemory = m_StagingBufferMemory, m_VertexBuffer = m_VertexBuffer,
				m_VertexBufferMemory = m_VertexBufferMemory]()
				{
					auto device = VulkanContext::GetLogicalDevice()->GetNative();

					// Destroy staging buffer
					vkDestroyBuffer(device, m_StagingBuffer, nullptr);
					vkFreeMemory(device, m_StagingBufferMemory, nullptr);

					// Destroy vertex buffer
					vkDestroyBuffer(device, m_VertexBuffer, nullptr);
					vkFreeMemory(device, m_VertexBufferMemory, nullptr);
				});
		}
	}

	void VertexBuffer::CopyData(VkBuffer src, VkBuffer dst, size_t size)
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
			copyRegion.size = size;
			vkCmdCopyBuffer(commandBuffer, src, dst, 1, &copyRegion);

			vkEndCommandBuffer(commandBuffer);
		}
		{
			VkFence submitFence{ VK_NULL_HANDLE };

			VkFenceCreateInfo fenceInfo{};
			fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			//fenceInfo.flags = VK_FENCE_CREATE_UNSIGNALED_BIT;

			MUR_VK_ASSERT(vkCreateFence(device->GetNative(), &fenceInfo, nullptr, &submitFence));

			VkSubmitInfo submitInfo{};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &commandBuffer;

			vkQueueSubmit(device->GetGraphicsQueue(), 1, &submitInfo, submitFence);

			vkWaitForFences(device->GetNative(), 1, &submitFence, VK_TRUE, UINT64_MAX);

			vkDestroyFence(device->GetNative(), submitFence, nullptr);
			vkFreeCommandBuffers(device->GetNative(), tempPool.GetNative(), 1, &commandBuffer);
		}
	}

	VertexBuffer::~VertexBuffer()
	{
	}

	void VertexBuffer::SetData(void* srcData, uint32_t size)
	{
		auto device = VulkanContext::GetLogicalDevice()->GetNative();

		// Set data into the staging buffer
		void* data;
		vkMapMemory(device, m_StagingBufferMemory, 0, size, 0, &data);
		memcpy(data, srcData, size);
		vkUnmapMemory(device, m_StagingBufferMemory);

		// Transfer into vertex
		CopyData(m_StagingBuffer, m_VertexBuffer, size);
	}

}