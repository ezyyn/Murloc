#include "pgpch.h"

#include "Texture.h"

#include "VulkanContext.h"
#include "VulkanUtils.h"
#include "VulkanBuffer.h"
#include "Pangolin/Renderer/Renderer2D.h"

#pragma warning(push, 0)
#include <stb_image.h>
#pragma warning(pop)

namespace H {
	static void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {

		auto device = PG::VulkanContext::GetLogicalDevice()->GetNative();

		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to create buffer!");
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = PG::Utils::FindMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate buffer memory!");
		}

		vkBindBufferMemory(device, buffer, bufferMemory, 0);
	}

	static void RecordTransititonImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, VkCommandBuffer commandBuffer)
	{
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else {
			throw std::invalid_argument("unsupported layout transition!");
		}

		vkCmdPipelineBarrier(
			commandBuffer,
			sourceStage, destinationStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);
	}

	static void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, VkCommandBuffer commandBuffer)
	{
		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = { width, height, 1};

		vkCmdCopyBufferToImage(
			commandBuffer,
			buffer,
			image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&region
		);
	}
}

namespace PG {

	Texture2D::Texture2D(const std::string& filepath)
		: m_FilePath(filepath), m_Width(0), m_Height(0)
	{
		m_TextureFormat = VK_FORMAT_R8G8B8A8_UNORM;
		// Loading texture to memory
		int texWidth{ 0 }, texHeight{ 0 }, texChannels{ 0 };

		//stbi_set_flip_vertically_on_load(1);
		uint8_t* pixels = stbi_load(m_FilePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

		m_Width = texWidth;
		m_Height = texHeight;
		PG_CORE_ASSERT(m_Width * m_Height * 4, "Invalid size of loaded picture!");

		CreateImage();
		Transfer(pixels, m_Width * m_Height * 4);
		stbi_image_free(pixels);
	}

	Texture2D::Texture2D(uint32_t color)
	{
		m_TextureFormat = VK_FORMAT_R8G8B8A8_UNORM;

		m_Width = 1;
		m_Height = 1;

		CreateImage();
		Transfer(&color, 4);
	}

	Texture2D::~Texture2D()
	{
	}

	void Texture2D::Invalidate()
	{

	}

	void Texture2D::Transfer(const void* pixels, size_t size)
	{
		auto device = VulkanContext::GetLogicalDevice()->GetNative();

		VulkanBuffer* stagingBuffer = nullptr;

		stagingBuffer = new VulkanBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		stagingBuffer->SetData(pixels);

		// Executing command buffers
		ScopeCommandPool tempPool;
		{
			// Pipeline barrier, transition image layout
			VkCommandBufferAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandPool = tempPool.GetNative();
			allocInfo.commandBufferCount = 1;

			VkCommandBuffer commandBuffer;
			vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			vkBeginCommandBuffer(commandBuffer, &beginInfo);
			{
				H::RecordTransititonImageLayout(m_TextureImage, m_TextureFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, commandBuffer);
			}
			vkEndCommandBuffer(commandBuffer);

			VkSubmitInfo submitInfo{};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &commandBuffer;

			vkQueueSubmit(VulkanContext::GetLogicalDevice()->GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
			vkQueueWaitIdle(VulkanContext::GetLogicalDevice()->GetGraphicsQueue());

			vkFreeCommandBuffers(device, tempPool.GetNative(), 1, &commandBuffer);
		}

		{
			// Copy buffer to image barrier
			// Copy buffer to image barrier
			// Copy buffer to image barrier
			VkCommandBufferAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandPool = tempPool.GetNative();
			allocInfo.commandBufferCount = 1;

			VkCommandBuffer commandBuffer;
			vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			vkBeginCommandBuffer(commandBuffer, &beginInfo);
			{
				H::CopyBufferToImage(stagingBuffer->GetBuffer(), m_TextureImage, m_Width, m_Height, commandBuffer);
			}
			vkEndCommandBuffer(commandBuffer);
			VkSubmitInfo submitInfo{};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &commandBuffer;
			vkQueueSubmit(VulkanContext::GetLogicalDevice()->GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
			vkQueueWaitIdle(VulkanContext::GetLogicalDevice()->GetGraphicsQueue());

			vkFreeCommandBuffers(device, tempPool.GetNative(), 1, &commandBuffer);
		}

		{
			// Pipeline barrier, transition image layout 2
			// Pipeline barrier, transition image layout 2
			// Pipeline barrier, transition image layout 2
			VkCommandBufferAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandPool = tempPool.GetNative();
			allocInfo.commandBufferCount = 1;

			VkCommandBuffer commandBuffer;
			vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			vkBeginCommandBuffer(commandBuffer, &beginInfo);
			{
				H::RecordTransititonImageLayout(m_TextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, commandBuffer);
			}
			vkEndCommandBuffer(commandBuffer);

			VkSubmitInfo submitInfo{};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &commandBuffer;

			vkQueueSubmit(VulkanContext::GetLogicalDevice()->GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
			vkQueueWaitIdle(VulkanContext::GetLogicalDevice()->GetGraphicsQueue());

			vkFreeCommandBuffers(device, tempPool.GetNative(), 1, &commandBuffer);
		}
		delete stagingBuffer;

		// Creating TextureImageView for texture
		// Creating TextureImageView for texture
		// Creating TextureImageView for texture
		CreateTextureImageView();
		CreateTextureSampler();
		//CreateDescriptorSets();

		// Resource free queue
		auto& resourceFreeQueue = VulkanContext::GetContextResourceFreeQueue();
		resourceFreeQueue.PushBack(CommandQueueExecutionPriority::SAMPLER,
			[device, m_TextureSampler = m_TextureSampler]
			{
				vkDestroySampler(device, m_TextureSampler, nullptr);
			});
		resourceFreeQueue.PushBack(CommandQueueExecutionPriority::IMAGEVIEW,
			[device, m_TextureImageView = m_TextureImageView]
			{
				vkDestroyImageView(device, m_TextureImageView, nullptr);
			});

		resourceFreeQueue.PushBack(CommandQueueExecutionPriority::IMAGE,
			[device, m_TextureImage = m_TextureImage, m_TextureImageMemory = m_TextureImageMemory]
			{
				vkDestroyImage(device, m_TextureImage, nullptr);
				vkFreeMemory(device, m_TextureImageMemory, nullptr);
			});
	}

	void Texture2D::CreateImage()
	{
		auto device = VulkanContext::GetLogicalDevice()->GetNative();

		// Creating image 
		// Creating image 
		// Creating image 
		{
			VkImageCreateInfo imageInfo{};
			imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageInfo.imageType = VK_IMAGE_TYPE_2D;
			imageInfo.extent.width = m_Width;
			imageInfo.extent.height = m_Height;
			imageInfo.extent.depth = 1;
			imageInfo.mipLevels = 1;
			imageInfo.arrayLayers = 1;
			imageInfo.format = m_TextureFormat; 
			imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
			imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			PG_VK_ASSERT(vkCreateImage(device, &imageInfo, nullptr, &m_TextureImage));

			VkMemoryRequirements memRequirements;
			vkGetImageMemoryRequirements(device, m_TextureImage, &memRequirements);

			VkMemoryAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = Utils::FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

			PG_VK_ASSERT(vkAllocateMemory(device, &allocInfo, nullptr, &m_TextureImageMemory));

			PG_VK_ASSERT(vkBindImageMemory(device, m_TextureImage, m_TextureImageMemory, 0));
		}
	}

	void Texture2D::CreateTextureImageView()
	{
		auto device = VulkanContext::GetLogicalDevice()->GetNative();

		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = m_TextureImage;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = m_TextureFormat;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		PG_VK_ASSERT(vkCreateImageView(device, &viewInfo, nullptr, &m_TextureImageView));
	}

	void Texture2D::CreateTextureSampler()
	{
		auto device = VulkanContext::GetLogicalDevice();

		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

		/*samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = device->GetPhysicalDevice().GetSupportDetails().Properties.limits.maxSamplerAnisotropy;*/
		samplerInfo.anisotropyEnable = VK_FALSE;
		samplerInfo.maxAnisotropy = 1.0f;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		PG_VK_ASSERT(vkCreateSampler(device->GetNative(), &samplerInfo, nullptr, &m_TextureSampler));
	}

/*
	void Texture2D::CreateDescriptorSets()
	{
		auto device = VulkanContext::GetLogicalDevice()->GetNative();
		auto framesInFlight = Renderer2D::GetSettings().FramesInFlight;
		{
			// Descriptor set layout binding
			// Descriptor set layout binding
			// Descriptor set layout binding
			VkDescriptorSetLayoutBinding samplerLayoutBinding{};
			samplerLayoutBinding.binding = 0;
			samplerLayoutBinding.descriptorCount = 1;
			samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			samplerLayoutBinding.pImmutableSamplers = nullptr;
			samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

			// Descriptor set layout
			// Descriptor set layout
			// Descriptor set layout
			std::array<VkDescriptorSetLayoutBinding, 1> bindings = { samplerLayoutBinding };
			VkDescriptorSetLayoutCreateInfo layoutInfo{};
			layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
			layoutInfo.pBindings = bindings.data();

			PG_VK_ASSERT(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &m_DescriptorSetLayout))
		}
		// Descriptor pool
		// Descriptor pool
		// Descriptor pool
		{
			std::array<VkDescriptorPoolSize, 1> poolSizes{};
			poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			poolSizes[0].descriptorCount = framesInFlight;

			VkDescriptorPoolCreateInfo poolInfo{};
			poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
			poolInfo.pPoolSizes = poolSizes.data();
			poolInfo.maxSets = framesInFlight;

			vkCreateDescriptorPool(device, &poolInfo, nullptr, &m_DescriptorPool);
		}
		{
			// Descriptor set allocate
			// Descriptor set allocate
			// Descriptor set allocate
			std::vector<VkDescriptorSetLayout> layouts(framesInFlight, m_DescriptorSetLayout);
			VkDescriptorSetAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = m_DescriptorPool;
			allocInfo.descriptorSetCount = framesInFlight;
			allocInfo.pSetLayouts = layouts.data();

			m_DescriptorSets.resize(framesInFlight);

			PG_VK_ASSERT(vkAllocateDescriptorSets(device, &allocInfo, m_DescriptorSets.data()));
		}

		for (size_t i = 0; i < framesInFlight; i++) {
			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = m_TextureImageView; 
			imageInfo.sampler = m_TextureSampler;

			VkWriteDescriptorSet descriptorWrites{};
			descriptorWrites.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites.dstSet = m_DescriptorSets[i];
			descriptorWrites.dstBinding = 1;
			descriptorWrites.dstArrayElement = 0;
			descriptorWrites.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites.descriptorCount = 1;
			descriptorWrites.pImageInfo = &imageInfo;

			vkUpdateDescriptorSets(device, 1, &descriptorWrites, 0, nullptr);
		}*/

}