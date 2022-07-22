#pragma once

#include <vulkan/vulkan.h>

namespace PG {

	struct Image2DInfo {
		VkFormat Format;
		VkMemoryPropertyFlags Storage;
		uint32_t Width;
		uint32_t Height;
		VkImageUsageFlags Usage;
	};

	class Image2D
	{
	public:
		Image2D(const Image2DInfo& info);
		~Image2D();

		void Invalidate(uint32_t width, uint32_t height);

		VkImageView GetImageView() const { return m_ImageView; }
		VkImage GetImage() const { return m_Image; }

		uint32_t GetWidth() const { return m_ImageInfo.Width; }
		uint32_t GetHeight() const { return m_ImageInfo.Height; }
	private:
		Image2DInfo m_ImageInfo;

		VkImage m_Image{ VK_NULL_HANDLE };
		VkImageView m_ImageView{ VK_NULL_HANDLE };
		VkDeviceMemory m_ImageMemory{ VK_NULL_HANDLE };
	};

}