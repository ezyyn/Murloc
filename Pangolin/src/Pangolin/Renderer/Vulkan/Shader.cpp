#include "pgpch.h"

#include "Shader.h"

#include "Pangolin/Core/Timer.h"

#include "VulkanContext.h"

#include <fstream>
#include <sstream>

#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_glsl.hpp>

namespace PG {

	namespace Utils {

		static ShaderStage ShaderTypeFromString(const std::string& type)
		{
			if (type == "vertex")
				return ShaderStage::VERTEX;
			if (type == "fragment" || type == "pixel")
				return ShaderStage::FRAGMENT;

			PG_CORE_ASSERT(false, "Unknown shader!");

			return ShaderStage::INVALID;
		}

		static shaderc_shader_kind GLShaderStageToShaderC(ShaderStage stage)
		{
			switch (stage)
			{
			case ShaderStage::VERTEX:   return shaderc_glsl_vertex_shader;
			case ShaderStage::FRAGMENT: return shaderc_glsl_fragment_shader;
			}

			PG_CORE_ASSERT(false, "");
			return (shaderc_shader_kind)0;
		}

		static VkShaderStageFlags GLShaderStageToVulkan(ShaderStage stage)
		{
			switch (stage)
			{
			case ShaderStage::VERTEX:   return VK_SHADER_STAGE_VERTEX_BIT;
			case ShaderStage::FRAGMENT: return VK_SHADER_STAGE_FRAGMENT_BIT;
			}

			PG_CORE_ASSERT(false, "");
			return (shaderc_shader_kind)0;
		}

		static const char* GLShaderStageToString(ShaderStage stage)
		{
			switch (stage)
			{
			case ShaderStage::VERTEX:   return "GL_VERTEX_SHADER";
			case ShaderStage::FRAGMENT: return "GL_FRAGMENT_SHADER";
			}
			PG_CORE_ASSERT("", false);
			return nullptr;
		}

		static const char* GLShaderStageCachedVulkanFileExtension(ShaderStage stage)
		{
			switch (stage)
			{
			case ShaderStage::VERTEX:    return ".cached_vulkan.vert";
			case ShaderStage::FRAGMENT:  return ".cached_vulkan.frag";
			}
			PG_CORE_ASSERT(false, "");
			return "";
		}

		static ShaderDataType ShaderCBaseToShaderDataType(spirv_cross::SPIRType::BaseType type)
		{
			switch (type)
			{
			case spirv_cross::SPIRType::BaseType::Float:    return ShaderDataType::Float;
			case spirv_cross::SPIRType::BaseType::Int:      return ShaderDataType::Int;
			}
			PG_CORE_ASSERT(false, "");
			return ShaderDataType::Float;
		}
		
		static VkFormat ShaderDataTypeToVulkanFormat(ShaderDataType type) {

			switch (type) {
			case ShaderDataType::Float:  return VK_FORMAT_R32_SFLOAT;
			case ShaderDataType::Float2: return VK_FORMAT_R32G32_SFLOAT;
			case ShaderDataType::Float3: return VK_FORMAT_R32G32B32_SFLOAT;
			case ShaderDataType::Float4: return VK_FORMAT_R32G32B32A32_SFLOAT;
			}

			PG_CORE_ASSERT(false);

			return VK_FORMAT_UNDEFINED;
		}
	}

	Shader::Shader(const std::string& filepath)
		: m_Filepath(filepath)
	{
		ReadAndPreprocess();
		// Whole shader up-to-date business
		CheckIfUpToDate();
		CompileOrGetVulkanBinaries();
		CreateShader();
		// GenerateDescriptors();
	}

	VkDescriptorSet Shader::GetCurrentDescriptorSet() const
	{
		return m_DescriptorSet;
	}

	void Shader::GenerateDescriptors()
	{
		auto device = VulkanContext::GetLogicalDevice()->GetNative();

		// Uniform Bindings
		// Uniform Bindings
		// Uniform Bindings
		std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings = {};
		for (auto& [binding, uniform] : m_UniformBufferMap)
		{
			auto& descriptorBinding = descriptorSetLayoutBindings.emplace_back();
			descriptorBinding = {};
			descriptorBinding.binding = binding;
			descriptorBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorBinding.descriptorCount = 1;
			descriptorBinding.stageFlags = uniform.Stage;
			descriptorBinding.pImmutableSamplers = nullptr; // Optional
		}

		// Samplers
		// Samplers
		// Samplers
		auto& descriptorBinding = descriptorSetLayoutBindings.emplace_back();
		descriptorBinding = {};
		descriptorBinding.binding = 1;
		descriptorBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorBinding.descriptorCount = m_Samplers.size();
		descriptorBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		descriptorBinding.pImmutableSamplers = nullptr; // Optional

		// Creating descriptor layout (pipeline)
		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(descriptorSetLayoutBindings.size());
		layoutInfo.pBindings = descriptorSetLayoutBindings.data();

		PG_VK_ASSERT(vkCreateDescriptorSetLayout(device,
			&layoutInfo, nullptr, m_DescriptorSetLayouts.data()));

		// Pool
		// Pool
		// Pool
		{
			// Pool Size
			// Pool	Size
			// Pool	Size
			std::vector<VkDescriptorPoolSize> poolSizes;

			poolSizes.push_back({ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, (uint32_t)m_UniformBufferMap.size() });
			poolSizes.push_back({ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, (uint32_t)m_Samplers.size() });

			// Pool
			// Pool
			// Pool
			VkDescriptorPoolCreateInfo poolInfo{};
			poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			poolInfo.poolSizeCount = (uint32_t)poolSizes.size();
			poolInfo.pPoolSizes = poolSizes.data();
			poolInfo.maxSets = 1;
			PG_VK_ASSERT(vkCreateDescriptorPool(device, &poolInfo, nullptr, &m_DescriptorPool));
		}

		// Allocating Descriptor Sets
		// Allocating Descriptor Sets
		// Allocating Descriptor Sets
		{
			VkDescriptorSetAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = m_DescriptorPool;
			allocInfo.descriptorSetCount = (uint32_t)m_DescriptorSetLayouts.size();
			allocInfo.pSetLayouts = m_DescriptorSetLayouts.data();

			PG_VK_ASSERT(vkAllocateDescriptorSets(device, &allocInfo, &m_DescriptorSet));
		}
		// Configuring DescriptorSets | Uniforms
		// Configuring DescriptorSets | Uniforms
		// Configuring DescriptorSets | Uniforms
		for (auto& [binding, uniform] : m_UniformBufferMap)
		{
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = uniform.Buffer->GetBuffer();
			bufferInfo.offset = 0;
			bufferInfo.range = uniform.Buffer->Size();

			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = m_DescriptorSet;
			descriptorWrite.dstBinding = binding;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &bufferInfo;

			vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
		}

		// Configuring DescriptorSets | Samplers
		// Configuring DescriptorSets | Samplers
		// Configuring DescriptorSets | Samplers

		for (size_t i = 0; i < m_Samplers.size(); ++i)
		{
			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = m_Samplers[i].ImageView;
			imageInfo.sampler = m_Samplers[i].Sampler;

			VkWriteDescriptorSet descriptorWrites[1]{};
			descriptorWrites[0] = {};
			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = m_DescriptorSet;
			descriptorWrites[0].dstBinding = m_Samplers[i].Binding;
			descriptorWrites[0].dstArrayElement = i;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pImageInfo = &imageInfo;

/*
			VkDescriptorImageInfo imageInfo2{};
			imageInfo2.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo2.imageView = m_Samplers[1].ImageView;
			imageInfo2.sampler = m_Samplers[1].Sampler;
			descriptorWrites[1] = {};
			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = m_DescriptorSet;
			descriptorWrites[1].dstBinding = m_Samplers[1].Binding;
			descriptorWrites[1].dstArrayElement = 1;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; //TODO: Probably rework whole shader descriptor thing
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pImageInfo = &imageInfo2;*/

			vkUpdateDescriptorSets(device, 1, descriptorWrites, 0, nullptr);
		}

		// Resource free queue
		auto& resourceFreeQueue = VulkanContext::GetContextResourceFreeQueue();

		resourceFreeQueue.PushBack(DESCRIPTOR_POOL, [device, m_DescriptorPool = m_DescriptorPool]()
			{
				vkDestroyDescriptorPool(device, m_DescriptorPool, nullptr);
			});

		resourceFreeQueue.PushBack(DESCRIPTOR_SET_LAYOUT, [device, &m_DescriptorSetLayouts = m_DescriptorSetLayouts]()
			{
				auto device = VulkanContext::GetLogicalDevice()->GetNative();
				for (auto layout : m_DescriptorSetLayouts)
				{
					vkDestroyDescriptorSetLayout(device, layout, nullptr);
				}
			});
	}

	void Shader::SetUniformData(uint32_t binding, const void* data)
	{
		m_UniformBufferMap.at(binding).Buffer->SetData(data);
	}

	void Shader::AddTextureIntoBinding(uint32_t binding, const Ref<Texture2D>& texture)
	{
		m_Samplers.push_back(Sampler{ binding, texture->GetTextureImageView(), texture->GetTextureSampler() });
		//PG_CORE_ASSERT(m_SamplerMap.count(binding) == 0);
		//m_Samplers[binding].Sampler = texture->GetTextureSampler();
		//m_Samplers[binding].ImageView = texture->GetTextureImageView();
	}

	Shader::~Shader()
	{
	}

	void Shader::CheckIfUpToDate()
	{
		ScopedTimer timer("Checking if shader is up-to-date");
		auto purifyString = [](char c) {
			return c == '\0' || c == '\r' || c == '\n';
		};

		// Creates or gets 
		// Metadata will contain hashed version of previous compiled version of shader
		// Filepath of metadata will be same as filepath to the actual shader
		std::string pathToMetadata = m_Filepath + ".metadata";
		std::string wholeShader;
		{
			std::ifstream in_WholeShaderStream(m_Filepath, std::ios::in);

			// Reads shader file 
			std::string line;
			std::stringstream wholeShaderStringStream;
			while (std::getline(in_WholeShaderStream, line))
			{
				wholeShaderStringStream << line;
			}

			// Purify
			wholeShader = wholeShaderStringStream.str();
			wholeShader.erase(std::remove_if(wholeShader.begin(), wholeShader.end(), purifyString), wholeShader.end());
		}

		std::ifstream in_MetaDataStream(pathToMetadata, std::ios::in);
		// Creates hash from the actual shader file
		uint64_t hashFromShaderFile = std::hash<std::string>()(wholeShader); 
		uint64_t hashFromShaderMetaDataFile = 0;
		// If metadata exists, open, read and create hash from it
		if (in_MetaDataStream.is_open())
		{
			// Reading from metadata
			std::stringstream wholeShaderFromMetaDataStream;

			std::string line;
			while (std::getline(in_MetaDataStream, line))
			{
				wholeShaderFromMetaDataStream << line;
			}
			// Purify
			std::string& wholeShaderFromMetaData = wholeShaderFromMetaDataStream.str();
			wholeShaderFromMetaData.erase(std::remove_if(wholeShaderFromMetaData.begin(), wholeShaderFromMetaData.end(), purifyString), wholeShaderFromMetaData.end());

			// Creating hash
			hashFromShaderMetaDataFile = std::hash<std::string>()(wholeShaderFromMetaData);
		}
		m_Compile = hashFromShaderFile != hashFromShaderMetaDataFile;


		if (m_Compile) {
			// If compilation is needed then (re)create shader metadata file
			std::ofstream out(pathToMetadata, std::ios::out | std::ios::trunc);
			PG_CORE_ASSERT(out.is_open());

			std::istringstream stream(wholeShader);
			std::string line;
			std::stringstream test;
			while (std::getline(stream, line))
			{
				test << line;
			}

			// Purify
			auto& testStr = test.str();
			testStr.erase(std::remove_if(testStr.begin(), testStr.end(), purifyString), testStr.end());

			out << testStr;

			out.flush();
			out.close();
		}
	}

	void Shader::ReadAndPreprocess()
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
				PG_CORE_ERROR("Could not read from file '{0}'", m_Filepath);
			}
		}
		else
		{
			PG_CORE_ERROR("Could not open file '{0}'", m_Filepath);
		}

		std::unordered_map<ShaderStage, std::string> shaderSources;

		const char* typeToken = "#type";
		size_t typeTokenLength = strlen(typeToken);
		size_t pos = sourceCode.find(typeToken, 0); //Start of shader type declaration line
		while (pos != std::string::npos)
		{
			size_t eol = sourceCode.find_first_of("\r\n", pos); //End of shader type declaration line
			PG_CORE_ASSERT(eol != std::string::npos, "Syntax error");
			size_t begin = pos + typeTokenLength + 1; //Start of shader type name (after "#type " keyword)
			std::string type = sourceCode.substr(begin, eol - begin);
			PG_CORE_ASSERT(Utils::ShaderTypeFromString(type) != ShaderStage::INVALID, "Invalid shader type specified");

			size_t nextLinePos = sourceCode.find_first_not_of("\r\n", eol); //Start of shader code after shader type declaration line
			PG_CORE_ASSERT(nextLinePos != std::string::npos, "Syntax error");
			pos = sourceCode.find(typeToken, nextLinePos); //Start of next shader type declaration line

			shaderSources[Utils::ShaderTypeFromString(type)] = (pos == std::string::npos) ? sourceCode.substr(nextLinePos) : sourceCode.substr(nextLinePos, pos - nextLinePos);
		}

		m_ShaderSources = shaderSources;
	}

	void Shader::CompileOrGetVulkanBinaries()
	{
		m_VulkanCompiled.clear();

		if (m_Compile) // Multithreaded compile
		{
			std::vector<std::thread> compileThreadPool;
			shaderc::Compiler compiler{};
			for (auto&& [stage, source] : m_ShaderSources)
			{
				// Apparently can compile multiple shaders simultaneously
				compileThreadPool.emplace_back(&Shader::CompileSingleStageJob, this, &compiler, source, stage);
			}

			for (auto& thread : compileThreadPool)
			{
				thread.join();
			}
		}
		else // Just retrieve cached shaders
		{
			std::filesystem::path cacheDirectory = "assets/shaders/cached";
			std::filesystem::path shaderFilePath = m_Filepath;

			for (auto&& [stage, source] : m_ShaderSources)
			{
				std::filesystem::path cachedPath = cacheDirectory / (shaderFilePath.filename().string()
					+ Utils::GLShaderStageCachedVulkanFileExtension(stage));

				std::ifstream in(cachedPath, std::ios::in | std::ios::binary);
				PG_CORE_WARN("[{0}] Shader {1} is up-to-date!", Utils::GLShaderStageToString(stage), m_Filepath);
				in.seekg(0, std::ios::end);
				auto size = in.tellg();
				in.seekg(0, std::ios::beg);

				auto& data = m_VulkanCompiled[stage];
				data.resize(size / sizeof(uint32_t));
				in.read((char*)data.data(), size);
			}
		}

		for (auto&& [stage, data] : m_VulkanCompiled)
			Reflect(stage, data);
	}

	void Shader::CompileSingleStageJob(shaderc::Compiler* compiler, const std::string source, ShaderStage stage)
	{
		ScopedTimer timer(std::string(Utils::GLShaderStageToString(stage)) + " Compilation");

		std::filesystem::path cacheDirectory = "assets/shaders/cached";
		std::filesystem::path shaderFilePath = m_Filepath;
		std::filesystem::path cachedPath = cacheDirectory / (shaderFilePath.filename().string() 
			+ Utils::GLShaderStageCachedVulkanFileExtension(stage));

		shaderc::CompileOptions options;

		options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);

#ifdef MUR_RELEASE
		const bool optimize = true;
#else 
		const bool optimize = false;
#endif

		if (optimize)
			options.SetOptimizationLevel(shaderc_optimization_level_performance);

		PG_CORE_WARN("[{0}] Shader {1} is not up-to-date! Compiling...", Utils::GLShaderStageToString(stage), m_Filepath);
		shaderc::SpvCompilationResult modulee = compiler->CompileGlslToSpv(source, 
			Utils::GLShaderStageToShaderC(stage), m_Filepath.c_str(), options);
		if (modulee.GetCompilationStatus() != shaderc_compilation_status_success)
		{
			PG_CORE_ERROR(modulee.GetErrorMessage());
			PG_CORE_ASSERT(false, "");
		}
		m_VulkanCompiled[stage] = std::vector<uint32_t>(modulee.cbegin(), modulee.cend());
		// This should overwrite the contents of previous cached shader
		std::ofstream out(cachedPath, std::ios::out | std::ios::binary | std::ios::trunc);
		if (out.is_open())
		{
			auto& data = m_VulkanCompiled[stage];
			out.write((char*)data.data(), data.size() * sizeof(uint32_t));
			out.flush();
			out.close();
		}
	}

	void Shader::Reflect(ShaderStage stage, const std::vector<uint32_t>& shaderData)
	{
		std::string& name = "Shader reflection[" + std::string(Utils::GLShaderStageToString(stage)) + "]";
		ScopedTimer timer(name);
		spirv_cross::Compiler compiler(shaderData);
		spirv_cross::ShaderResources& resources = compiler.get_shader_resources();

		PG_CORE_INFO("GLSLShader::Reflect - {0} {1}", Utils::GLShaderStageToString(stage), m_Filepath);
		PG_CORE_TRACE("    {0} uniform buffer(s)", resources.uniform_buffers.size());
		PG_CORE_TRACE("    {0} resource(s)", resources.sampled_images.size());

		// Uniform buffers
		// Uniform buffers
		// Uniform buffers
		PG_CORE_TRACE("Uniform buffers:");
		for (const auto& resource : resources.uniform_buffers)
		{
			const auto& bufferType = compiler.get_type(resource.base_type_id);
			uint32_t bufferSize = (uint32_t)compiler.get_declared_struct_size(bufferType);
			uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			size_t uniformMemberCount = bufferType.member_types.size();

			PG_CORE_ERROR("  {0}", resource.name);
			PG_CORE_TRACE("    Size = {0}", bufferSize);
			PG_CORE_TRACE("    Binding = {0}", binding);
			PG_CORE_TRACE("    Member(s) = {0}", uniformMemberCount);

			// Create uniform buffers
			{
				auto device = VulkanContext::GetLogicalDevice()->GetNative();

				auto& uniformBuffer = m_UniformBufferMap[binding];
				uniformBuffer.Buffer = CreateRef<VulkanBuffer>(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
				uniformBuffer.Stage = Utils::GLShaderStageToVulkan(stage);
			}
		}
		PG_CORE_TRACE("===============================================");

		if (stage == ShaderStage::VERTEX) {
			PG_CORE_TRACE("Attributes:");
			// Attribute Inputs
			// Attribute Inputs
			// Attribute Inputs
			// Compare lambda 
			auto asceningOrderLambda = [&compiler]
			(spirv_cross::Resource& a, spirv_cross::Resource& b)
			{
				uint32_t location1 = compiler.get_decoration(a.id, spv::DecorationLocation);
				uint32_t location2 = compiler.get_decoration(b.id, spv::DecorationLocation);

				return location1 < location2;
			};
			// Sort attributes by location
			// NOTE: For some reason stage inputs are not in the order as they are written in shader.
			std::sort(resources.stage_inputs.begin(), resources.stage_inputs.end(), asceningOrderLambda);
			
			// Local var for offsetting attributes in the buffer
			uint32_t attributeOffset = 0;
			// Loop through every attribute found in the shader(they are now sorted)
			for (const auto& resource : resources.stage_inputs)
			{
				const auto& bufferType = compiler.get_type(resource.base_type_id);
				uint32_t location = compiler.get_decoration(resource.id, spv::DecorationLocation);
				uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
				// const auto& name = compiler.get_name(resource.id); // Does not work for some reason
				// Query for type of the attribute
				const auto& type = compiler.get_type_from_variable(resource.id);
				// Calculating actual size of the attribute with base type of @type and 
				uint32_t actualDataTypeSize = (uint32_t)Utils::ShaderCBaseToShaderDataType(type.basetype) * (uint32_t)type.vecsize;
				// Printing out useful informations(debug only)
				PG_CORE_ERROR("  {0}", resource.name);
				PG_CORE_TRACE("    Location = {0}", location);
				PG_CORE_TRACE("    Size = {0} byte(s)", actualDataTypeSize);

				VkVertexInputAttributeDescription attributeDesc{};
				attributeDesc.binding = binding; // Not sure
				attributeDesc.format = Utils::ShaderDataTypeToVulkanFormat((ShaderDataType)actualDataTypeSize);
				attributeDesc.location = location;
				attributeDesc.offset = attributeOffset;
				//PG_CORE_ERROR("Offset 2: {0}", attributeOffset);

				attributeOffset += (uint32_t)actualDataTypeSize;

				m_VertexBufferLayout.AddAttributeDescription(attributeDesc, actualDataTypeSize);
			}
		} 
		else if (stage == ShaderStage::FRAGMENT) 
		{
			PG_CORE_TRACE("Samplers: ");
			for (const auto& resource : resources.sampled_images)
			{
				const auto& bufferType = compiler.get_type(resource.type_id);
				uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
				size_t arraySize = bufferType.array[0]; 
				PG_CORE_ERROR("  {0}[{1}]", resource.name, arraySize);
				PG_CORE_TRACE("    Binding = {0}", binding);
			}
		}
		PG_CORE_INFO("===============================================");
	}

	void Shader::CreateShader()
	{
		auto device = VulkanContext::GetLogicalDevice()->GetNative();

		for (auto& [stage, code] : m_VulkanCompiled) {

			VkShaderModuleCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

			createInfo.codeSize = code.size() * sizeof(uint32_t);
			createInfo.pCode = code.data();

			PG_VK_ASSERT(vkCreateShaderModule(device, &createInfo, nullptr, &m_Shaders[stage]));
		}

		// Resource free queue
		auto& resourceFreeQueue = VulkanContext::GetContextResourceFreeQueue();

		resourceFreeQueue.PushBack(SHADER_MODULE, [device, &m_Shaders = m_Shaders]()
			{
				for (auto& [stage, shader] : m_Shaders) {
					vkDestroyShaderModule(device, shader, nullptr);
				}
			});
	}
}