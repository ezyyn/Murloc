#pragma once

#include <vulkan/vulkan.h>

#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_glsl.hpp>

namespace Murloc {

	enum class ShaderType {
		INVALID = -1,
		VERTEX = 0,
		FRAGMENT = 1
		/*GEOMETRY,
		TESSELATION*/
	};

	class VulkanShader {
	public:
		VulkanShader(const std::string& filepath);

		std::unordered_map<ShaderType, VkShaderModule>& GetShaderModules() { return m_Shaders; };

		~VulkanShader();
	private:
		void ReadAndPreprocess();
		void CompileOrGetVulkanBinaries();
		void Reflect(ShaderType stage, const std::vector<uint32_t>& shaderData);
		void CreateShader();

		std::unordered_map<ShaderType, VkShaderModule> m_Shaders;
		std::unordered_map<ShaderType, std::string> m_ShaderSources;
		std::unordered_map<ShaderType, std::vector<uint32_t>> m_VulkanCompiled;

		std::string m_Filepath;
	};

}