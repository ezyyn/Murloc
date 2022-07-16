#include "murpch.hpp"

#include "ShaderLibrary.h"
#include "Murloc/Core/Timer.hpp"

#include <fstream>
#include <yaml-cpp/yaml.h>

#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_glsl.hpp>

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

	ShaderLibrary ShaderLibrary::s_Instance;

	Ref<Shader> ShaderLibrary::LoadOrGet(const char* shaderName)
	{
		auto& shaderData = s_Instance.LoadOrGet_Impl(shaderName);

		return nullptr;
	}

	void ShaderLibrary::Init_Impl(const std::string& filepath)
	{
		m_ShaderLibraryFilePath = filepath + "/.ShaderLibrary"; // - assets/shaders/.ShaderLibrary

		auto& textureShaderData = m_ShaderLibrary["Texture"];

		textureShaderData.FilePath = "assets/shaders/Shader_Texture/Texture.glsl";
		CheckUpToDate(textureShaderData);
		Compile(textureShaderData);

		// Read yaml if exists
		if (!std::filesystem::exists(m_ShaderLibraryFilePath))
		{
			YAML::Emitter out;
			out << YAML::BeginMap;
			out << YAML::Key << "Shaders" << YAML::Value << YAML::BeginSeq;
			{
				out << YAML::BeginMap; // Shader
				out << YAML::Key << "Shader" << YAML::Value << textureShaderData.FilePath;
				out << YAML::Key << "Hash" << textureShaderData.PreviouslyHashedSources;
				out << YAML::EndMap; // Shader
			}
			out << YAML::EndSeq;
			out << YAML::EndMap;

			std::ofstream fout(m_ShaderLibraryFilePath, std::ios_base::trunc);
			fout << out.c_str();

			return;
		}

		YAML::Node data;
		try
		{
			data = YAML::LoadFile(m_ShaderLibraryFilePath);
		}
		catch (YAML::ParserException e)
		{
			MUR_CORE_ASSERT(false);
		}
	}

	const ShaderData& ShaderLibrary::LoadOrGet_Impl(const char* shaderName)
	{
		// Check if exists, if yes, check if it is up-to-date
		if (m_ShaderLibrary.count(shaderName))
		{
			// Shader is in the library
			// Check if up-to-date
			auto& shaderData = m_ShaderLibrary[shaderName];

			if (CheckUpToDate(shaderData))
			{
				// Up-to-date
				return shaderData;
			}
			else {
				// Recompile
				// Update values in yaml

				Compile(shaderData);
				UpdateYAML(shaderData);
			}
		}
		
		MUR_CORE_ASSERT(false);
		return m_ShaderLibrary[shaderName];
	}

	bool ShaderLibrary::CheckUpToDate(ShaderData& shaderData)
	{
		// Read file
		std::string sourceCode;
		std::ifstream in(shaderData.FolderFilePath + "/" + shaderData.FilePath, std::ios::in | std::ios::binary); // ifstream closes itself due to RAII
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
				MUR_CORE_ERROR("Could not read from file '{0}'", m_ShaderLibraryFilePath);
			}
		}
		else
		{
			MUR_CORE_ERROR("Could not open file '{0}'", m_ShaderLibraryFilePath);
		}
		// hash string
		std::hash<std::string> hashString;
		size_t hashedString = hashString(sourceCode);

		size_t previouslyHashedSources = shaderData.PreviouslyHashedSources;

		shaderData.PreviouslyHashedSources = hashedString;
		return previouslyHashedSources == hashedString;
	}

	void ShaderLibrary::Compile(ShaderData& shaderData)
	{
		MUR_CORE_WARN("GENERATING SHADER: {0}", shaderData.FilePath);

		ScopedTimer timer("Shader compilation");

		std::filesystem::path cacheDirectory = shaderData.FolderFilePath; // assets/shaders/Shader_Texture

		shaderc::Compiler compiler{};

		shaderc::CompileOptions options;
		options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);

		const bool optimize = true;
		if (optimize)
			options.SetOptimizationLevel(shaderc_optimization_level_performance);

		shaderData.Sources.clear();
		for (auto&& [stage, source] : m_ShaderSources)
		{
			std::filesystem::path shaderFilePath = shaderData.FilePath;
			std::filesystem::path cachedPath = cacheDirectory / (shaderFilePath.filename().string() + Utils::GLShaderStageCachedVulkanFileExtension(stage));

			std::ifstream in(cachedPath, std::ios::in | std::ios::binary);
			if (in.is_open())
			{
				in.seekg(0, std::ios::end);
				auto size = in.tellg();
				in.seekg(0, std::ios::beg);

				auto& data = shaderData.Sources[stage];
				data.resize(size / sizeof(uint32_t));
				in.read((char*)data.data(), size);
			}
			else
			{
				shaderc::SpvCompilationResult modulee = compiler.CompileGlslToSpv(source, Utils::GLShaderStageToShaderC(stage), shaderData.FilePath.c_str(), options);
				if (modulee.GetCompilationStatus() != shaderc_compilation_status_success)
				{
					MUR_CORE_ERROR(modulee.GetErrorMessage());
					MUR_CORE_ASSERT(false, "");
				}

				shaderData.CompiledSources[stage] = std::vector<uint32_t>(modulee.cbegin(), modulee.cend());

				std::ofstream out(cachedPath, std::ios::out | std::ios::binary);
				if (out.is_open())
				{
					auto& data = shaderData.Sources[stage];
					out.write((char*)data.data(), data.size() * sizeof(uint32_t));
					out.flush();
					out.close();
				}
			}
		}

		for (auto&& [stage, data] : shaderData.CompiledSources)
			Reflect(stage, data, shaderData.FilePath);
	}

	void ShaderLibrary::UpdateYAML(ShaderData& shaderData)
	{
		// Yaml update or create

		YAML::Emitter out;
		{
			out << YAML::BeginMap; // Shader
			out << YAML::Key << "Shader" << YAML::Value << shaderData.FilePath; 
			out << YAML::Key << "Hash" << shaderData.PreviouslyHashedSources;
			out << YAML::EndMap; // Shader
		}
	}

	void ShaderLibrary::Reflect(ShaderType stage, const std::vector<uint32_t> compiled, const std::string& filepath)
	{
		spirv_cross::Compiler compiler(compiled);
		spirv_cross::ShaderResources resources = compiler.get_shader_resources();

		MUR_CORE_TRACE("GLSLShader::Reflect - {0} {1}", Utils::GLShaderStageToString(stage), filepath);
		MUR_CORE_TRACE("    {0} uniform buffers", resources.uniform_buffers.size());
		MUR_CORE_TRACE("    {0} resources", resources.sampled_images.size());

		MUR_CORE_TRACE("Uniform buffers:");
		for (const auto& resource : resources.uniform_buffers)
		{
			const auto& bufferType = compiler.get_type(resource.base_type_id);
			size_t bufferSize = compiler.get_declared_struct_size(bufferType);
			size_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			size_t memberCount = bufferType.member_types.size();

			MUR_CORE_TRACE("  {0}", resource.name);
			MUR_CORE_TRACE("    Size = {0}", bufferSize);
			MUR_CORE_TRACE("    Binding = {0}", binding);
			MUR_CORE_TRACE("    Members = {0}", memberCount);
		}
	}

	void ShaderLibrary::ReadAndCheckIfUpToDate()
	{
		std::string sourceCode;
		std::ifstream in(m_ShaderLibraryFilePath, std::ios::in | std::ios::binary); // ifstream closes itself due to RAII
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
				MUR_CORE_ERROR("Could not read from file '{0}'", m_ShaderLibraryFilePath);
			}
		}
		else
		{
			MUR_CORE_ERROR("Could not open file '{0}'", m_ShaderLibraryFilePath);
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

}