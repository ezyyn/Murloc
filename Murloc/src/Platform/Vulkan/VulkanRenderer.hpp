#pragma once

#include "VulkanVertexBuffer.hpp"

namespace Murloc {

	class VulkanRenderer {
	public:
		static void Init();
		static void Shutdown();

		static void OnResize(uint32_t width, uint32_t height);

		static void WaitIdle();

		static void Begin();
		static void Render(const Ref<VulkanVertexBuffer>& vertexBuffer, uint32_t count);
	private:

	};
}