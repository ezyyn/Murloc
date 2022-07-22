#pragma once

#include <vulkan/vulkan.h>

namespace PG {

	enum class ShaderDataType : uint32_t { // in bytes
		Float  = 4 * 1,
		Float2 = 4 * 2,
		Float3 = 4 * 3,
		Float4 = 4 * 4,

		Int  = 4 * 1,
		Int2 = 4 * 2,
		Int3 = 4 * 3,
		Int4 = 4 * 4,
	};

	class BufferLayout {
	public:
		BufferLayout() {}
		~BufferLayout() {}

		uint32_t GetStride() const { return m_Stride; }
		uint32_t Size() const { return (uint32_t)m_AttributesDescriptions.size(); }

		void AddAttributeDescription(VkVertexInputAttributeDescription description, uint32_t size);

		const std::vector<VkVertexInputAttributeDescription>& GetAttributesDescription() const { return m_AttributesDescriptions; };
	private:
		void CalculateOffsetAndStride();

		uint32_t m_Stride{ 0 };

		std::vector<VkVertexInputAttributeDescription> m_AttributesDescriptions;
	};

}
