#pragma once

#include <vulkan/vulkan.h>

#include "Pangolin/Renderer/Vulkan/Image2D.h"

namespace PG {

	struct Texture2DInfo {
		std::string FilePath;
		bool Transfer;

		// Core info
		VkFormat Format;
		VkMemoryPropertyFlags Storage;
		uint32_t Width;
		uint32_t Height;
		VkImageUsageFlags Usage;
	};

	class Texture
	{
	public:
		Texture();
		~Texture();
	private:

	};

	// Used for loading and storing texture on GPU
	class Texture2D {
	public:
		Texture2D(const std::string& filepath);
		Texture2D(uint32_t color);
		~Texture2D();

		VkSampler GetTextureSampler() const { return m_TextureSampler; }
		VkImageView GetTextureImageView() const { return m_TextureImageView; }

		void Invalidate();
	private:
		void Transfer(const void* pixels, size_t size);

		void CreateImage();
		void CreateTextureImageView();
		void CreateTextureSampler();

		VkSampler m_TextureSampler;
		VkFormat m_TextureFormat;

		VkImageView m_TextureImageView;
		VkImage m_TextureImage;
		VkDeviceMemory m_TextureImageMemory;

		uint32_t m_Width, m_Height;
		std::string m_FilePath;
	};
}