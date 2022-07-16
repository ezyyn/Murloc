#pragma once

#include <vulkan/vulkan.h>

#include "Murloc/Renderer/BufferLayout.hpp"
#include "VulkanBuffer.h"

namespace Murloc {

	enum class ShaderType {
		INVALID = -1,
		VERTEX = 0,
		FRAGMENT = 1
		/*GEOMETRY,
		TESSELATION*/
	};

	struct UniformBuffer {
		Ref<VulkanBuffer> Buffer;
		VkShaderStageFlags Stage;
		uint32_t Binding;
	};

	struct UniformBufferInfo {
		uint32_t Binding;
		VkShaderStageFlags Stage;
		uint32_t Size;
	};

	class Shader {
	public:
		Shader(const std::string& filepath);
		~Shader();

		const BufferLayout& GetBufferLayout() { return m_BufferLayout; }

		std::unordered_map<ShaderType, VkShaderModule>& GetShaderModules() { return m_Shaders; };

		const std::vector<VkDescriptorSetLayout>& GetDescriptorSetLayouts() const { return m_DescriptorSetLayouts; }

		VkDescriptorSet GetCurrentDescriptorSet() const;

		[[nodiscard]] std::vector<UniformBuffer> GetAllUniformBuffers()
		{
			std::vector<UniformBuffer> buffers;
			buffers.resize(m_UniformBufferMap.size());
			for (auto& [binding, buffer] : m_UniformBufferMap) {
				buffers.emplace_back(buffer);
			}

			return buffers;
		}

		void SetUniformData(uint32_t binding, const void* data);
	private:
		void ReadAndPreprocess();
		void CompileOrGetVulkanBinaries();
		void Reflect(ShaderType stage, const std::vector<uint32_t>& shaderData);
		void CreateShader();
		// Creates Descriptors and manages them into one descriptor layout
		void CreateDescriptors();

		BufferLayout m_BufferLayout;

		std::unordered_map<ShaderType, VkShaderModule> m_Shaders;
		std::unordered_map<ShaderType, std::string> m_ShaderSources;
		std::unordered_map<ShaderType, std::vector<uint32_t>> m_VulkanCompiled;

		std::unordered_map<uint32_t, UniformBuffer> m_UniformBufferMap; // Binding and uniform buffer
		VkDescriptorPool m_DescriptorPool{ VK_NULL_HANDLE };
		VkDescriptorSet m_DescriptorSet{ VK_NULL_HANDLE };
		//std::vector<Ref<VulkanBuffer>> m_UniformBuffers;
		
		//std::vector<VkDescriptorSet> m_DescriptorSets{ VK_NULL_HANDLE };

		std::vector<VkDescriptorSetLayout> m_DescriptorSetLayouts{ VK_NULL_HANDLE };

		std::string m_Filepath;
	};

}