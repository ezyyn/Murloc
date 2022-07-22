#include "pgpch.h"

#include "VertexBuffer.h"

#include "VulkanContext.h"
#include "VulkanUtils.h"
#define Pangolin_VULKAN_FUNCTIONS
#include "Pangolin/Renderer/RenderManager.h"
#include "Pangolin/Renderer/Vulkan/Swapchain.h"

#include "Pangolin/Core/Timer.h"

namespace PG {

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

			PG_VK_ASSERT(vkCreateBuffer(device, &bufferInfo, nullptr, &m_VertexBuffer));

			VkMemoryRequirements memRequirements;
			vkGetBufferMemoryRequirements(device, m_VertexBuffer, &memRequirements);

			VkMemoryAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = memRequirements.size;
			// VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT - Memory allocated with this type can be mapped for host access using vkMapMemory
			allocInfo.memoryTypeIndex = Utils::FindMemoryType(memRequirements.memoryTypeBits,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT); // VRAM ?

			PG_VK_ASSERT(vkAllocateMemory(device, &allocInfo, nullptr, &m_VertexBufferMemory));

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

			PG_VK_ASSERT(vkCreateBuffer(device, &bufferInfo, nullptr, &m_StagingBuffer));

			VkMemoryRequirements memRequirements;
			vkGetBufferMemoryRequirements(device, m_StagingBuffer, &memRequirements);

			VkMemoryAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = Utils::FindMemoryType(memRequirements.memoryTypeBits,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

			PG_VK_ASSERT(vkAllocateMemory(device, &allocInfo, nullptr, &m_StagingBufferMemory));

			vkBindBufferMemory(device, m_StagingBuffer, m_StagingBufferMemory, 0);

			vkMapMemory(device, m_StagingBufferMemory, 0, size, 0, &m_StagingBufferMapPtr);
		}

		// CopyData command buffer
		{
			//ScopeCommandPool tempPool; // For short-live cmd buffers

			VkCommandBufferAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
			allocInfo.commandPool = VulkanContext::GetLogicalDevice()->GetCommandPool();
			allocInfo.commandBufferCount = RenderManager::GetSettings().FramesInFlight;

			m_CopyDataCmdBuffers.resize(RenderManager::GetSettings().FramesInFlight);
			vkAllocateCommandBuffers(device, &allocInfo, m_CopyDataCmdBuffers.data());
		}
		// Swapchain resource free queue
		{
			auto& resourceFreeQueue = VulkanContext::GetContextResourceFreeQueue();

			resourceFreeQueue.PushBack(BUFFER, [m_StagingBuffer = m_StagingBuffer,
				m_StagingBufferMemory = m_StagingBufferMemory, m_VertexBuffer = m_VertexBuffer,
				m_VertexBufferMemory = m_VertexBufferMemory]()
				{
					auto device = VulkanContext::GetLogicalDevice()->GetNative();
					// Unmapping
					vkUnmapMemory(device, m_StagingBufferMemory);

					// Destroy staging buffer
					vkDestroyBuffer(device, m_StagingBuffer, nullptr);
					vkFreeMemory(device, m_StagingBufferMemory, nullptr);

					// Destroy vertex buffer
					vkDestroyBuffer(device, m_VertexBuffer, nullptr);
					vkFreeMemory(device, m_VertexBufferMemory, nullptr);
				});
		}
	}

	void VertexBuffer::CopyData(VkBuffer src, VkBuffer dst, size_t size, uint32_t currentFrame)
	{
		auto device = VulkanContext::GetLogicalDevice();

		// Should be zero 

		//RenderManager::SubmitSecondary([&m_CopyDataCmdBuffers = m_CopyDataCmdBuffers, src, dst, size](uint32_t currentFrame, uint32_t imageIndex)
			{
				VkCommandBufferInheritanceInfo inheritanceInfo{};
				inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;

				VkCommandBufferBeginInfo beginInfo{};
				beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
				beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
				beginInfo.pInheritanceInfo = &inheritanceInfo;

				vkBeginCommandBuffer(m_CopyDataCmdBuffers[currentFrame], &beginInfo);

				VkBufferCopy copyRegion{};
				copyRegion.srcOffset = 0; // Optional
				copyRegion.dstOffset = 0; // Optional
				copyRegion.size = size;
				vkCmdCopyBuffer(m_CopyDataCmdBuffers[currentFrame], src, dst, 1, &copyRegion);

				vkEndCommandBuffer(m_CopyDataCmdBuffers[currentFrame]);

				// Returns secondary buffer we want to submit
				//return m_CopyDataCmdBuffers[currentFrame];
			}
//		);
	}

	VertexBuffer::~VertexBuffer()
	{
	}

	void VertexBuffer::SetData(void* srcData, uint32_t size, uint32_t currentFrame)
	{
		auto device = VulkanContext::GetLogicalDevice()->GetNative();

		// Copy data into the staging buffer 
		memcpy(m_StagingBufferMapPtr, srcData, size);

		// Transfer into vertex
		CopyData(m_StagingBuffer, m_VertexBuffer, size, currentFrame);
	}
}