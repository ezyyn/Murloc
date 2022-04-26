#pragma once

#include <vulkan/vulkan.h>

namespace Murloc {

	enum class ShaderDataType : uint32_t { // in bytes
		Float  = 4 * 1,
		Float2 = 4 * 2,
		Float3 = 4 * 3,
		Float4 = 4 * 4,
	};

	struct VertexAttribute {
		ShaderDataType DataType;
		std::string Name;
	};

	class BufferLayout {
	public:
		BufferLayout(const std::initializer_list<VertexAttribute>& attributes);
		~BufferLayout() {}

		uint32_t NumberOfAttributes() const { return m_VertexAttributes.size(); }

		const VkVertexInputBindingDescription& GetBindingDesc() const { return m_BindingDesc; }
		const std::vector<VkVertexInputAttributeDescription> GetAttributesDesc() const { return m_AttributesDesc; }
	private:
		VkVertexInputBindingDescription m_BindingDesc;
		std::vector<VkVertexInputAttributeDescription> m_AttributesDesc;
		std::vector<VertexAttribute> m_VertexAttributes;
	};

}
