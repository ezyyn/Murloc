#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan.h> // TODO: REMOVE DEBUG ONLY

namespace Murloc {

	struct QuadVertex {

		glm::vec3 Position;
		glm::vec4 Color;

		static VkVertexInputBindingDescription getBindingDescription() {
			VkVertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(QuadVertex);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return bindingDescription;
		}

		static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
			std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(QuadVertex, Position);

			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(QuadVertex, Color);

			return attributeDescriptions;
		}
	};

	class Renderer {
	public:
		static void Init();
		static void Shutdown();

		static void OnResize(uint32_t width, uint32_t height);

		static void BeginFrame();
		static void EndFrame();
	private:

	};
}