#include "murpch.hpp"

#include "Shader.h"

#include "Murloc/Core/Timer.hpp"

#include "VulkanContext.h"
#include "Swapchain.h"
#include "Murloc/Renderer/Renderer.hpp"

#include <fstream>

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

		static VkShaderStageFlags GLShaderStageToVulkan(ShaderType stage)
		{
			switch (stage)
			{
			case ShaderType::VERTEX:   return VK_SHADER_STAGE_VERTEX_BIT;
			case ShaderType::FRAGMENT: return VK_SHADER_STAGE_FRAGMENT_BIT;
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

		static ShaderDataType ShaderCBaseToShaderDataType(spirv_cross::SPIRType::BaseType type)
		{
			switch (type)
			{
			case spirv_cross::SPIRType::BaseType::Float:    return ShaderDataType::Float;
			case spirv_cross::SPIRType::BaseType::Int:      return ShaderDataType::Int;
			}
			MUR_CORE_ASSERT(false, "");
			return ShaderDataType::Float;
		}
		
		static VkFormat ShaderDataTypeToVulkanFormat(ShaderDataType type) {

			switch (type) {
			case ShaderDataType::Float:  return VK_FORMAT_R32_SFLOAT;
			case ShaderDataType::Float2: return VK_FORMAT_R32G32_SFLOAT;
			case ShaderDataType::Float3: return VK_FORMAT_R32G32B32_SFLOAT;
			case ShaderDataType::Float4: return VK_FORMAT_R32G32B32A32_SFLOAT;
			}

			MUR_CORE_ASSERT(false);

			return VK_FORMAT_UNDEFINED;
		}
	}

	/*struct UniformBufferData {
		glm::mat4 Model;
		glm::mat4 View;
		glm::mat4 Projection;
	};*/ // (16 + 16 + 16) * 4

	Shader::Shader(const std::string& filepath)
		: m_Filepath(filepath)
	{
		ReadAndPreprocess();
		CompileOrGetVulkanBinaries();
		CreateShader();
		CreateDescriptors();
	}

	VkDescriptorSet Shader::GetCurrentDescriptorSet() const
	{
		return m_DescriptorSet;
	}

	void Shader::CreateDescriptors()
	{
		auto device = VulkanContext::GetLogicalDevice()->GetNative();

		// Uniform Bindings
		// Uniform Bindings
		// Uniform Bindings
		std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings = { /*samplerLayoutBinding*/ };
		for (auto& [name, uniform] : m_UniformBufferMap)
		{
			auto& descriptorBinding = descriptorSetLayoutBindings.emplace_back();
			descriptorBinding = {};
			descriptorBinding.binding = uniform.Binding;
			descriptorBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorBinding.descriptorCount = 1;
			descriptorBinding.stageFlags = uniform.Stage;
			descriptorBinding.pImmutableSamplers = nullptr; // Optional
		}

		// Creating descriptor layout (pipeline)
		// Creating descriptor layout (pipeline)
		// Creating descriptor layout (pipeline)

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(descriptorSetLayoutBindings.size());
		layoutInfo.pBindings = descriptorSetLayoutBindings.data();

		MUR_VK_ASSERT(vkCreateDescriptorSetLayout(device,
			&layoutInfo, nullptr, m_DescriptorSetLayouts.data()));

		// Pool
		// Pool
		// Pool
		{
			// Pool Size
			// Pool	Size
			// Pool	Size
			std::vector<VkDescriptorPoolSize> poolSizes;
			poolSizes.resize(m_UniformBufferMap.size());

			for (size_t i = 0; i < poolSizes.size(); ++i)
			{
				poolSizes[i].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				poolSizes[i].descriptorCount = 1;
			}

			/*poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			poolSizes[1].descriptorCount = Renderer::GetSettings().FramesInFlight;*/
			// Pool
			// Pool
			// Pool
			VkDescriptorPoolCreateInfo poolInfo{};
			poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			poolInfo.poolSizeCount = (uint32_t)poolSizes.size();
			poolInfo.pPoolSizes = poolSizes.data();
			poolInfo.maxSets = 1;
			MUR_VK_ASSERT(vkCreateDescriptorPool(device, &poolInfo, nullptr, &m_DescriptorPool));
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

			MUR_VK_ASSERT(vkAllocateDescriptorSets(device, &allocInfo, &m_DescriptorSet));
		}
		// Configuring DescriptorSets | Uniforms
		// Configuring DescriptorSets | Uniforms
		// Configuring DescriptorSets | Uniforms
		for (auto& [name, uniform] : m_UniformBufferMap)
		{
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = uniform.Buffer->GetBuffer();
			bufferInfo.offset = 0;
			bufferInfo.range = uniform.Buffer->Size();

			/*VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = s_Data.TestTexture->m_TextureImageView;
			imageInfo.sampler = s_Data.TestTexture->m_TextureSampler;*/

			VkWriteDescriptorSet descriptorWrite{};

			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = m_DescriptorSet;
			descriptorWrite.dstBinding = uniform.Binding;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &bufferInfo;

			/*descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = m_DescriptorSets[i];
			descriptorWrites[1].dstBinding = 1;
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pImageInfo = &imageInfo;*/

			vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
		}

		// Configuring DescriptorSets | Samplers
		// Configuring DescriptorSets | Samplers
		// Configuring DescriptorSets | Samplers
		{

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

	Shader::~Shader()
	{
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

	void Shader::CompileOrGetVulkanBinaries()
	{
		MUR_CORE_WARN("GENERATING SHADER: {0}", m_Filepath);

		ScopedTimer timer("Shader compilation");

		std::filesystem::path cacheDirectory = "assets/shaders/cached";

		shaderc::Compiler compiler{};

		shaderc::CompileOptions options;
		options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);

		const bool optimize = true;
		if (optimize)
			options.SetOptimizationLevel(shaderc_optimization_level_performance);

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
				shaderc::SpvCompilationResult modulee = compiler.CompileGlslToSpv(source, Utils::GLShaderStageToShaderC(stage), m_Filepath.c_str(), options);
				if (modulee.GetCompilationStatus() != shaderc_compilation_status_success)
				{
					MUR_CORE_ERROR(modulee.GetErrorMessage());
					MUR_CORE_ASSERT(false, "");
				}

				shaderData[stage] = std::vector<uint32_t>(modulee.cbegin(), modulee.cend());

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

	void Shader::Reflect(ShaderType stage, const std::vector<uint32_t>& shaderData)
	{
		spirv_cross::Compiler compiler(shaderData);
		spirv_cross::ShaderResources resources = compiler.get_shader_resources();

		MUR_CORE_TRACE("GLSLShader::Reflect - {0} {1}", Utils::GLShaderStageToString(stage), m_Filepath);
		MUR_CORE_TRACE("    {0} uniform buffer(s)", resources.uniform_buffers.size());
		MUR_CORE_TRACE("    {0} resource(s)", resources.sampled_images.size());

		MUR_CORE_TRACE("Uniform buffers:");
		for (const auto& resource : resources.uniform_buffers)
		{
			const auto& bufferType = compiler.get_type(resource.base_type_id);
			uint32_t bufferSize = (uint32_t)compiler.get_declared_struct_size(bufferType);
			uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			size_t uniformMemberCount = bufferType.member_types.size();

			MUR_CORE_TRACE("  {0}", resource.name);
			MUR_CORE_TRACE("    Size = {0}", bufferSize);
			MUR_CORE_TRACE("    Binding = {0}", binding);
			MUR_CORE_TRACE("    Member(s) = {0}", uniformMemberCount);

			// Create uniform buffers
			{
				auto device = VulkanContext::GetLogicalDevice()->GetNative();

				auto& uniformBuffer = m_UniformBufferMap[binding];
				uniformBuffer.Buffer = CreateRef<VulkanBuffer>(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
				uniformBuffer.Stage = Utils::GLShaderStageToVulkan(stage);
				uniformBuffer.Binding = binding;
			}
		}

		MUR_CORE_TRACE("Attributes:");
		for (const auto& resource : resources.stage_inputs)
		{
			const auto& bufferType = compiler.get_type(resource.base_type_id);
			uint32_t location = compiler.get_decoration(resource.id, spv::DecorationLocation);
			const auto& name = compiler.get_name(resource.id);

			const auto& type = compiler.get_type_from_variable(resource.id);

			MUR_CORE_TRACE("  {0}", name);
			MUR_CORE_TRACE("    Location = {0}", location);
			MUR_CORE_TRACE("    Size = {0}", type.vecsize);
		}

		if (stage == ShaderType::VERTEX) {
			uint32_t attributeOffset = 0;

			auto comparison = [&compiler]
			(spirv_cross::Resource& a, spirv_cross::Resource& b)
			{
				uint32_t location1 = compiler.get_decoration(a.id, spv::DecorationLocation);
				uint32_t location2 = compiler.get_decoration(b.id, spv::DecorationLocation);

				return location1 < location2;
			};
			std::sort(resources.stage_inputs.begin(), resources.stage_inputs.end(), comparison);

			for (const auto& resource : resources.stage_inputs)
			{
				const auto& bufferType = compiler.get_type(resource.base_type_id);
				uint32_t location = compiler.get_decoration(resource.id, spv::DecorationLocation);
				const auto& name = compiler.get_name(resource.id);
				const auto& type = compiler.get_type_from_variable(resource.id);

				uint32_t actualDataTypeSize = (uint32_t)Utils::ShaderCBaseToShaderDataType(type.basetype) * (uint32_t)type.vecsize;

				VkVertexInputAttributeDescription attributeDesc{};
				attributeDesc.binding = 0;
				attributeDesc.format = Utils::ShaderDataTypeToVulkanFormat((ShaderDataType)actualDataTypeSize);
				attributeDesc.location = location;
				attributeDesc.offset = attributeOffset;
				MUR_CORE_ERROR("Offset 2: {0}", attributeOffset);

				attributeOffset += (uint32_t)actualDataTypeSize;

				m_BufferLayout.AddAttributeDescription(attributeDesc, actualDataTypeSize);
			}
			/*m_VertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			VkVertexInputBindingDescription bindingDescription{};
			{
				bindingDescription.binding = 0;
				bindingDescription.stride = attributeOffset;
				MUR_CORE_ERROR("Stride 2: {0}", attributeOffset);
				bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
				m_VertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
				m_VertexInputStateCreateInfo.pVertexBindingDescriptions = &bindingDescription; // Optional
			}
			m_VertexInputStateCreateInfo.vertexAttributeDescriptionCount = resources.stage_inputs.size();
			m_VertexInputStateCreateInfo.pVertexAttributeDescriptions = attributesDescs.data(); // Optional*/
		}
	}

	void Shader::CreateShader()
	{
		auto device = VulkanContext::GetLogicalDevice()->GetNative();

		for (auto& [stage, code] : m_VulkanCompiled) {

			VkShaderModuleCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

			createInfo.codeSize = code.size() * sizeof(uint32_t);
			createInfo.pCode = code.data();

			MUR_VK_ASSERT(vkCreateShaderModule(device, &createInfo, nullptr, &m_Shaders[stage]));
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
/*
	void Shader::ManageDescriptors(const std::vector<VkDescriptorSetLayoutBinding>& bindings)
	{
		// Descriptor pool
		// Descriptor pool
		// Descriptor pool
		{
			std::array<VkDescriptorPoolSize, 1> poolSizes{};
			poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			poolSizes[0].descriptorCount = Renderer::GetSettings().FramesInFlight;
			/ *poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			poolSizes[1].descriptorCount = Renderer::GetSettings().FramesInFlight;* /

			VkDescriptorPoolCreateInfo poolInfo{};
			poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
			poolInfo.pPoolSizes = poolSizes.data();
			poolInfo.maxSets = 1;

			MUR_VK_ASSERT(vkCreateDescriptorPool(device, &poolInfo, nullptr, &m_DescriptorPool));
		}

		// Descriptor sets
		// Descriptor sets
		// Descriptor sets
		{
			uint32_t framesInFlight = / *Renderer::GetSettings().FramesInFlight* /1;

			std::vector<VkDescriptorSetLayout> layouts(framesInFlight, m_DescriptorSetLayouts[0]);
			VkDescriptorSetAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = m_DescriptorPool;
			allocInfo.descriptorSetCount = framesInFlight;
			allocInfo.pSetLayouts = layouts.data();

			m_DescriptorSets.resize(framesInFlight);
			MUR_VK_ASSERT(vkAllocateDescriptorSets(device, &allocInfo, m_DescriptorSets.data()));

			// Configuring DescriptorSets
			// Configuring DescriptorSets
			// Configuring DescriptorSets
			for (auto& buffer : m_UniformBufferMap)
			{
				for (uint32_t i = 0; i < framesInFlight; i++) {
					VkDescriptorBufferInfo bufferInfo{};
					bufferInfo.buffer = buffer.second->GetBuffer();
					bufferInfo.offset = 0;
					bufferInfo.range = buffer.second->Size();

					VkDescriptorImageInfo imageInfo{};
					imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					imageInfo.imageView = s_Data.TestTexture->m_TextureImageView;
					imageInfo.sampler = s_Data.TestTexture->m_TextureSampler;* /

					std::array<VkWriteDescriptorSet, 1> descriptorWrites{};

					descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					descriptorWrites[0].dstSet = m_DescriptorSets[i];
					descriptorWrites[0].dstBinding = 0;
					descriptorWrites[0].dstArrayElement = 0;
					descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
					descriptorWrites[0].descriptorCount = 1;
					descriptorWrites[0].pBufferInfo = &bufferInfo;

					descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					descriptorWrites[1].dstSet = m_DescriptorSets[i];
					descriptorWrites[1].dstBinding = 1;
					descriptorWrites[1].dstArrayElement = 0;
					descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					descriptorWrites[1].descriptorCount = 1;
					descriptorWrites[1].pImageInfo = &imageInfo;

					vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
				}
			}
		}
	}*/
}