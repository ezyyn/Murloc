#pragma once

#include <vulkan/vulkan.h>

#include "Pangolin/Renderer/BufferLayout.h"
#include "VulkanBuffer.h"
#include "Texture.h"

namespace shaderc {
	class Compiler;
	class CompileOptions;
}

namespace PG {

	enum class ShaderStage {
		INVALID = -1,
		VERTEX = 0,
		FRAGMENT = 1
		/*GEOMETRY,
		TESSELATION*/
	};

	struct UniformBuffer {
		Ref<VulkanBuffer> Buffer;
		VkShaderStageFlags Stage;
	};

	struct Sampler {
		uint32_t Binding;
		VkImageView ImageView;
		VkSampler Sampler;
	};

	class Shader {
	public:
		Shader(const std::string& filepath);
		~Shader();

		const BufferLayout& GetVertexBufferLayout() const { return m_VertexBufferLayout; }

		std::unordered_map<ShaderStage, VkShaderModule>& GetShaderModules() { return m_Shaders; };

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

		void AddTextureIntoBinding(uint32_t binding, const Ref<Texture2D>& sampler);

		// Creates Descriptors and manages them into one descriptor layout
		void GenerateDescriptors();
	private:
		void CheckIfUpToDate();
		void ReadAndPreprocess();
		void CompileOrGetVulkanBinaries();
		void CompileSingleStageJob(shaderc::Compiler* compiler, const std::string source, ShaderStage stage);
		void Reflect(ShaderStage stage, const std::vector<uint32_t>& shaderData);
		void CreateShader();
		

		BufferLayout m_VertexBufferLayout;

		bool m_Compile{ false };
		std::unordered_map<ShaderStage, VkShaderModule> m_Shaders;
		std::unordered_map<ShaderStage, std::string> m_ShaderSources;
		std::unordered_map<ShaderStage, std::vector<uint32_t>> m_VulkanCompiled;

		std::unordered_map<uint32_t, UniformBuffer> m_UniformBufferMap; // Binding and uniform buffer
		std::vector<Sampler> m_Samplers; // Binding and sampler
		VkDescriptorPool m_DescriptorPool{ VK_NULL_HANDLE };
		VkDescriptorSet m_DescriptorSet{ VK_NULL_HANDLE };
		//std::vector<Ref<VulkanBuffer>> m_UniformBuffers;
		
		//std::vector<VkDescriptorSet> m_DescriptorSets{ VK_NULL_HANDLE };

		std::vector<VkDescriptorSetLayout> m_DescriptorSetLayouts{ VK_NULL_HANDLE };
		
		std::string m_Filepath;
	};

}