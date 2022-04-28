#include "murpch.hpp"

#include "VulkanRenderer.hpp"

#include "Vulkan.hpp"
#include "VulkanCommandBuffers.hpp"
#include "VulkanUtils.hpp"
#include "VulkanPhysicalDevice.hpp"
#include "VulkanDevice.hpp"
#include "VulkanSwapchain.hpp"
#include "VulkanPipeline.hpp"
#include "VulkanShader.hpp"
#include "VulkanPipeline.hpp"

#include "Murloc/Renderer/BufferLayout.hpp"

#include "Murloc/Core/Application.hpp"

namespace Murloc {

	struct RenderObjects {
		// Beware of RAII, order matters
		Ref<VulkanSwapchain> Swapchain;
		Ref<VulkanShader> Shader;
		Ref<VulkanRenderPass> RenderPass;
		Ref<VulkanPipeline> GraphicsPipeline;

	};

	static RenderObjects* s_Objects{ nullptr };

	void VulkanRenderer::Init()
	{
		s_Objects = new RenderObjects;

		Vulkan::Init();

		s_Objects->Shader = CreateRef<VulkanShader>("assets/shaders/FlatColor.glsl");

		s_Objects->RenderPass = CreateRef<VulkanRenderPass>();

		s_Objects->Swapchain = CreateRef<VulkanSwapchain>(s_Objects->RenderPass->GetNative());

		BufferLayout quadBufferLayout = {
			{ShaderDataType::Float3, "a_Position"},
			{ShaderDataType::Float4, "a_Color"}
		};

		s_Objects->GraphicsPipeline = CreateRef<VulkanPipeline>(s_Objects->Shader, s_Objects->RenderPass, quadBufferLayout);
	}

	void VulkanRenderer::Shutdown()
	{
		delete s_Objects;
		Vulkan::Shutdown();
	}
	void VulkanRenderer::RecreateGraphicsPipeline()
	{
		// CPU wait for GPU to finish everything
		vkDeviceWaitIdle(Vulkan::GetDevice()->GetNative());

		// Clean up 
		uint32_t width = Application::Get()->GetWindow()->GetWidth();
		uint32_t height = Application::Get()->GetWindow()->GetHeight();

		MUR_CORE_WARN("RECREATING GRAPHICS PIPELINE {0}, {1}", width, height);

		// Recreate RenderPass
		s_Objects->RenderPass->CleanupAndRecreate(width, height);
		// Recreate SwapChain
		// Recreate ImageViews
		// Recreate Framebuffers
		s_Objects->Swapchain->CleanupAndRecreate(width, height, s_Objects->RenderPass->GetNative());
		// Recreate GraphicsPipeline
		s_Objects->GraphicsPipeline->CleanupAndRecreate(s_Objects->RenderPass);
	}

	VkRenderPass VulkanRenderer::GetRenderPass()
	{
		return s_Objects->RenderPass->GetNative();
	}

	VkCommandBuffer VulkanRenderer::GetCurrentDrawCommandBuffer()
	{
		MUR_CORE_ASSERT(false);
		return /*s_Objects->CommandBuffers->GetNative(s_CurrentFrame)*/nullptr;
	}

	const Ref<VulkanSwapchain>& VulkanRenderer::GetSwapchain()
	{
		return s_Objects->Swapchain;
	}

	void VulkanRenderer::WaitIdle()
	{
		vkDeviceWaitIdle(Vulkan::GetDevice()->GetNative());
	}

	void VulkanRenderer::DrawIndexed(const Ref<VulkanVertexBuffer>& vertexBuffer, const Ref<VulkanIndexBuffer>& indexBuffer, uint32_t count)
	{
		bool rebuildSwapchain = s_Objects->Swapchain->NewFrame();

		if (rebuildSwapchain) {

			RecreateGraphicsPipeline();
			return;
		}

		auto currentCommandBuffer = s_Objects->Swapchain->GetCurrentRenderCmdBuffer();
		auto currentFramebuffer = s_Objects->Swapchain->GetCurrentSwapchainFramebuffer();

		s_Objects->Swapchain->RecordCmdBuffer([&](VkCommandBuffer currentCommandBuffer)
			{
				s_Objects->RenderPass->Begin(currentFramebuffer, currentCommandBuffer);

				{
					VkExtent2D extent = s_Objects->Swapchain->GetSwapChainExtent();

					// Dynamic viewport
					VkViewport viewport{};
					viewport.x = 0.0f;
					viewport.y = 0.0f;
					viewport.width = (float)extent.width;
					viewport.height = (float)extent.height;
					viewport.minDepth = 0.0f;
					viewport.maxDepth = 1.0f;
					VkRect2D scissor{};
					scissor.offset = { 0, 0 };
					scissor.extent = extent;
					vkCmdSetViewport(currentCommandBuffer, 0, 1, &viewport);
					vkCmdSetScissor(currentCommandBuffer, 0, 1, &scissor);
				}

				s_Objects->GraphicsPipeline->Bind(currentCommandBuffer);

				VkBuffer vertexBuffers[]{ vertexBuffer->GetNative() };
				VkDeviceSize offsets[] = { 0 };
				vkCmdBindVertexBuffers(currentCommandBuffer, 0, 1, vertexBuffers, offsets);
				vkCmdBindIndexBuffer(currentCommandBuffer, indexBuffer->GetNative(), 0, VK_INDEX_TYPE_UINT32);

				vkCmdDrawIndexed(currentCommandBuffer, count, 1, 0, 0, 0);
				s_Objects->RenderPass->End(currentCommandBuffer);
			});
		// Execute command buffer
		s_Objects->Swapchain->Submit();

		// swaps frame and presents swapchain to the monitor
		rebuildSwapchain = s_Objects->Swapchain->EndFrame();

		if (rebuildSwapchain) {
			RecreateGraphicsPipeline();
		}

	}
}