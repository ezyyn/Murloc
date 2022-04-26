#include "murpch.hpp"

#include "VulkanShader.hpp"

#include "Murloc/Core/Timer.hpp"

#include "Vulkan.hpp"

#include <fstream>

namespace Murloc {

	namespace Utils {

		static ShaderType ShaderTypeFromString(const std::string& type)
		{
			if (type == "vertex")
				return ShaderType::VERTEX;
			if (type == "fragment" || type == "pixel")
				return ShaderType::FRAGMENT;

			MUR_CORE_ASSERT(false, "Unknown shader!");

			return ShaderType::INVALID;
		}

		static shaderc_shader_kind GLShaderStageToShaderC(ShaderType stage)
		{
			switch (stage)
			{
			case ShaderType::VERTEX:   return shaderc_glsl_vertex_shader;
			case ShaderType::FRAGMENT: return shaderc_glsl_fragment_shader;
			}

			MUR_CORE_ASSERT(false, "");
			return (shaderc_shader_kind)0;
		}

		static const char* GLShaderStageToString(ShaderType stage)
		{
			switch (stage)
			{
			case ShaderType::VERTEX:   return "GL_VERTEX_SHADER";
			case ShaderType::FRAGMENT: return "GL_FRAGMENT_SHADER";
			}
			MUR_CORE_ASSERT("", false);
			return nullptr;
		}

		static const char* GLShaderStageCachedVulkanFileExtension(ShaderType stage)
		{
			switch (stage)
			{
			case ShaderType::VERTEX:    return ".cached_vulkan.vert";
			case ShaderType::FRAGMENT:  return ".cached_vulkan.frag";
			}
			MUR_CORE_ASSERT(false, "");
			return "";
		}


	}

	VulkanShader::VulkanShader(const std::string& filepath)
		: m_Filepath(filepath)
	{
		ReadAndPreprocess();
		CompileOrGetVulkanBinaries();
		CreateShader();
	}

	VulkanShader::~VulkanShader()
	{
		for (auto& [stage, shader] : m_Shaders) {
			vkDestroyShaderModule(Vulkan::GetDevice()->GetNative(), shader, nullptr);
		}
	}

	void VulkanShader::ReadAndPreprocess()
	{
		std::string sourceCode;
		std::ifstream in(m_Filepath, std::ios::in | std::ios::binary); // ifstream closes itself due to RAII
		if (in)
		{
			in.seekg(0, std::ios::end);
			size_t size = in.tellg();
			if (size != -1)
			{
				sourceCode.resize(size);
				in.seekg(0, std::ios::beg);
				in.read(&sourceCode[0], size);
			}
			else
			{
				MUR_CORE_ERROR("Could not read from file '{0}'", m_Filepath);
			}
		}
		else
		{
			MUR_CORE_ERROR("Could not open file '{0}'", m_Filepath);
		}

		std::unordered_map<ShaderType, std::string> shaderSources;

		const char* typeToken = "#type";
		size_t typeTokenLength = strlen(typeToken);
		size_t pos = sourceCode.find(typeToken, 0); //Start of shader type declaration line
		while (pos != std::string::npos)
		{
			size_t eol = sourceCode.find_first_of("\r\n", pos); //End of shader type declaration line
			MUR_CORE_ASSERT(eol != std::string::npos, "Syntax error");
			size_t begin = pos + typeTokenLength + 1; //Start of shader type name (after "#type " keyword)
			std::string type = sourceCode.substr(begin, eol - begin);
			MUR_CORE_ASSERT(Utils::ShaderTypeFromString(type) != ShaderType::INVALID, "Invalid shader type specified");

			size_t nextLinePos = sourceCode.find_first_not_of("\r\n", eol); //Start of shader code after shader type declaration line
			MUR_CORE_ASSERT(nextLinePos != std::string::npos, "Syntax error");
			pos = sourceCode.find(typeToken, nextLinePos); //Start of next shader type declaration line

			shaderSources[Utils::ShaderTypeFromString(type)] = (pos == std::string::npos) ? sourceCode.substr(nextLinePos) : sourceCode.substr(nextLinePos, pos - nextLinePos);
		}

		m_ShaderSources = shaderSources;
	}

	void VulkanShader::CompileOrGetVulkanBinaries()
	{
		ScopedTimer timer("Shader compilation");

		shaderc::Compiler compiler{};

		shaderc::CompileOptions options;
		options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);

		const bool optimize = true;
		if (optimize)
			options.SetOptimizationLevel(shaderc_optimization_level_performance);

		std::filesystem::path cacheDirectory = "assets/shaders/cached";

		auto& shaderData = m_VulkanCompiled;
		shaderData.clear();
		for (auto&& [stage, source] : m_ShaderSources)
		{
			std::filesystem::path shaderFilePath = m_Filepath;
			std::filesystem::path cachedPath = cacheDirectory / (shaderFilePath.filename().string() + Utils::GLShaderStageCachedVulkanFileExtension(stage));

			std::ifstream in(cachedPath, std::ios::in | std::ios::binary);
			if (in.is_open())
			{
				in.seekg(0, std::ios::end);
				auto size = in.tellg();
				in.seekg(0, std::ios::beg);

				auto& data = shaderData[stage];
				data.resize(size / sizeof(uint32_t));
				in.read((char*)data.data(), size);
			}
			else
			{
				shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(source, Utils::GLShaderStageToShaderC(stage), m_Filepath.c_str(), options);
				if (module.GetCompilationStatus() != shaderc_compilation_status_success)
				{
					MUR_CORE_ERROR(module.GetErrorMessage());
					MUR_CORE_ASSERT(false, "");
				}

				shaderData[stage] = std::vector<uint32_t>(module.cbegin(), module.cend());

				std::ofstream out(cachedPath, std::ios::out | std::ios::binary);
				if (out.is_open())
				{
					auto& data = shaderData[stage];
					out.write((char*)data.data(), data.size() * sizeof(uint32_t));
					out.flush();
					out.close();
				}
			}
		}

		for (auto&& [stage, data] : shaderData)
			Reflect(stage, data);
	}

	void VulkanShader::Reflect(ShaderType stage, const std::vector<uint32_t>& shaderData)
	{
		spirv_cross::Compiler compiler(shaderData);
		spirv_cross::ShaderResources resources = compiler.get_shader_resources();

		MUR_CORE_TRACE("GLSLShader::Reflect - {0} {1}", Utils::GLShaderStageToString(stage), m_Filepath);
		MUR_CORE_TRACE("    {0} uniform buffers", resources.uniform_buffers.size());
		MUR_CORE_TRACE("    {0} resources", resources.sampled_images.size());

		MUR_CORE_TRACE("Uniform buffers:");
		for (const auto& resource : resources.uniform_buffers)
		{
			const auto& bufferType = compiler.get_type(resource.base_type_id);
			uint32_t bufferSize = compiler.get_declared_struct_size(bufferType);
			uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			int memberCount = bufferType.member_types.size();

			MUR_CORE_TRACE("  {0}", resource.name);
			MUR_CORE_TRACE("    Size = {0}", bufferSize);
			MUR_CORE_TRACE("    Binding = {0}", binding);
			MUR_CORE_TRACE("    Members = {0}", memberCount);
		}
	}

	void VulkanShader::CreateShader()
	{
		for (auto& [stage, code] : m_VulkanCompiled) {

			VkShaderModuleCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

			createInfo.codeSize = code.size() * sizeof(uint32_t);
			createInfo.pCode = code.data();

			MUR_VK_ASSERT(vkCreateShaderModule(Vulkan::GetDevice()->GetNative(), &createInfo, nullptr, &m_Shaders[stage]));
		}
	}

}