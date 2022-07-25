#include "pgpch.h"
#include "Image2D.h"

#include "VulkanUtils.h"
#include "VulkanContext.h"

namespace PG {

	Image2D::Image2D(const Image2DInfo& info)
		: m_ImageInfo(info)
	{
		auto device = VulkanContext::GetLogicalDevice()->GetNative();

		Invalidate(info.Width, info.Height);

		// Resource Free Queue
		auto& resourceFreeQueue = VulkanContext::GetContextResourceFreeQueue();

		resourceFreeQueue.PushBack(IMAGE, [device, m_Image = &m_Image, m_ImageMemory = &m_ImageMemory]()
			{
				vkDestroyImage(device, *m_Image, 0);
				vkFreeMemory(device, *m_ImageMemory, 0);
			});
		resourceFreeQueue.PushBack(IMAGEVIEW, [device, m_ImageView = &m_ImageView]()
			{
				vkDestroyImageView(device, *m_ImageView, 0);
			});
	}

	Image2D::~Image2D()
	{
	}

	void Image2D::Invalidate(uint32_t width, uint32_t height)
	{
		auto device = VulkanContext::GetLogicalDevice()->GetNative();
		// Destroy previous image
		if (m_Image)
		{
			vkDestroyImageView(device, m_ImageView, 0);
			vkDestroyImage(device, m_Image, 0);
			vkFreeMemory(device, m_ImageMemory, 0);

			m_Image = VK_NULL_HANDLE;
			m_ImageMemory = VK_NULL_HANDLE;
			m_ImageView = VK_NULL_HANDLE;
		}

		m_ImageInfo.Width = width;
		m_ImageInfo.Height = height;

		// Image
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = m_ImageInfo.Width;
		imageInfo.extent.height = m_ImageInfo.Height;
		imageInfo.usage = m_ImageInfo.Usage;
		imageInfo.format = m_ImageInfo.Format;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		// VK_IMAGE_LAYOUT_UNDEFINED or VK_IMAGE_LAYOUT_PREINITIALIZED
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		PG_VK_ASSERT(vkCreateImage(device, &imageInfo, nullptr, &m_Image));

		// Image memory
		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device, m_Image, &memRequirements);
		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = Utils::FindMemoryType(memRequirements.memoryTypeBits, m_ImageInfo.Storage);

		PG_VK_ASSERT(vkAllocateMemory(device, &allocInfo, nullptr, &m_ImageMemory));
		PG_VK_ASSERT(vkBindImageMemory(device, m_Image, m_ImageMemory, 0));

		// Image view
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = m_Image;
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = m_ImageInfo.Format;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		VkImageSubresourceRange image_range = { m_ImageInfo.Aspect, 0, 1, 0, 1 };
		createInfo.subresourceRange = image_range;
		PG_VK_ASSERT(vkCreateImageView(device, &createInfo, nullptr, &m_ImageView));

	}

}