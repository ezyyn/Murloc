#include "pgpch.h"

#include "BufferLayout.h"

namespace PG {

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

	void BufferLayout::AddAttributeDescription(VkVertexInputAttributeDescription description, uint32_t size)
	{
		m_AttributesDescriptions.emplace_back(description);
		m_Stride += size;
	}

	void BufferLayout::CalculateOffsetAndStride()
	{
		m_Stride = 0;

		uint32_t location = 0;
		for (auto& attribute : m_AttributesDescriptions) {
			
			PG_CORE_ERROR("Offset: {0}", m_Stride);
		}
		PG_CORE_ERROR("Stride: {0}", m_Stride);

	}

}