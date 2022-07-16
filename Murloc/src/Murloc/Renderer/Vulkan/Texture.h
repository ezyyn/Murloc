#pragma once

#include <vulkan/vulkan.h>

namespace Murloc {
	class Texture {
	public:
		Texture(const std::string& filepath);
		~Texture();

		VkDescriptorSetLayout GetDescriptorSetLayout() const { return m_DescriptorSetLayout; }

		std::vector<VkDescriptorSet>& GetDescriptorSets() { return m_DescriptorSets; }

		VkSampler m_TextureSampler;
		VkImageView m_TextureImageView;

	private:
		void CreateImage();
		void CreateTextureImageView();
		void CreateTextureSampler();
		void CreateDescriptorSets();

		std::vector<VkDescriptorSet> m_DescriptorSets;
		
		VkDescriptorPool m_DescriptorPool;
		VkDescriptorSetLayout m_DescriptorSetLayout;
		
		uint32_t m_Width, m_Height;

		VkImage m_TextureImage;
		VkDeviceMemory m_TextureImageMemory;
		std::string m_FilePath;
	};
}