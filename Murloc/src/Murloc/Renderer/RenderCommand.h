#pragma once

#include "Murloc/Renderer/Vulkan/VertexBuffer.h"
#include "Murloc/Renderer/Vulkan/IndexBuffer.h"

#include "Murloc/Renderer/Vulkan/GraphicsPipeline.h"

namespace Murloc {

	class RenderCommand {
	public:
		static void SetClearValue(VkClearValue value);
		static void WaitIdle();
		static void DrawIndexed(const Ref<VertexBuffer>& vertexBuffer, const Ref<IndexBuffer>& indexBuffer, const Ref<GraphicsPipeline>& pipeline, const Ref<Shader>& shader, uint32_t count);
	};

}
