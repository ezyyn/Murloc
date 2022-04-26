#include "murpch.hpp"

#include "BufferLayout.hpp"

namespace Murloc {

	namespace Utils {

		static VkFormat ShaderDataTypeToVulkanFormat(ShaderDataType type) {

			switch (type) {
				case ShaderDataType::Float:  return VK_FORMAT_R32_SFLOAT;
				case ShaderDataType::Float2: return VK_FORMAT_R32G32_SFLOAT;
				case ShaderDataType::Float3: return VK_FORMAT_R32G32B32_SFLOAT;
				case ShaderDataType::Float4: return VK_FORMAT_R32G32B32A32_SFLOAT;
			}

			__debugbreak();

		}

	}

	BufferLayout::BufferLayout(const std::initializer_list<VertexAttribute>& attributes)
		: m_VertexAttributes(attributes.begin(), attributes.end())
	{
		uint32_t stride = 0;

		for (auto& attribute : m_VertexAttributes) {

			stride += (uint32_t)attribute.DataType;

		}
		m_BindingDesc.binding = 0;
		m_BindingDesc.stride = stride;
		m_BindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		uint32_t location = 0;
		uint32_t offset = 0;
		for (auto attribute : m_VertexAttributes) {
			VkVertexInputAttributeDescription attributeDesc{};
			attributeDesc.binding = 0;
			attributeDesc.format = Utils::ShaderDataTypeToVulkanFormat(attribute.DataType);
			attributeDesc.location = location;
			attributeDesc.offset = offset;

			m_AttributesDesc.emplace_back(attributeDesc);

			offset += (uint32_t)attribute.DataType;
			location++;
		}
	}

}