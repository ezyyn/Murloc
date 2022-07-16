#include "murpch.hpp"

#include "RenderCommand.h"

#include <vulkan/vulkan.h>

#include "Murloc/Renderer/Vulkan/Swapchain.h"

#include "Murloc/Core/Application.hpp"

#include "Murloc/Renderer/Vulkan/VulkanContext.h"

namespace Murloc {

	struct RenderState {
		VkClearValue ClearValue{ 0 };
	};

	static RenderState s_RenderState;

	void RenderCommand::SetClearValue(VkClearValue value)
	{
		s_RenderState.ClearValue = value;
	}

	void RenderCommand::WaitIdle()
	{
		auto device = VulkanContext::GetLogicalDevice();
		vkDeviceWaitIdle(device->GetNative());
	}

	void RenderCommand::DrawIndexed(const Ref<VertexBuffer>& vertexBuffer, 
		const Ref<IndexBuffer>& indexBuffer, const Ref<GraphicsPipeline>& pipeline, const Ref<Shader>& shader, uint32_t count)
	{
		auto currentRenderCmdBuffer = Swapchain::s_CurrentRenderCommandbuffer;
		auto currentFramebuffer = Swapchain::s_CurrentFramebuffer;

		auto& appSpec = Application::Get()->GetSpecification();
		VkExtent2D extent = { appSpec.Width, appSpec.Height };

		// CommandBuffers
		// CommandBuffers
		// CommandBuffers
		{
			vkResetCommandBuffer(currentRenderCmdBuffer, 0);

			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // Optional
			beginInfo.pInheritanceInfo = nullptr; // Optional
			MUR_VK_ASSERT(vkBeginCommandBuffer(currentRenderCmdBuffer, &beginInfo));
		}

		// RenderPass Begin
		// RenderPass Begin
		// RenderPass Begin
		{
			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = Swapchain::s_RenderPass;
			renderPassInfo.framebuffer = currentFramebuffer;
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = extent;

			renderPassInfo.clearValueCount = 1;
			renderPassInfo.pClearValues = &s_RenderState.ClearValue;
			vkCmdBeginRenderPass(currentRenderCmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		}

		{
			{
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
				vkCmdSetViewport(currentRenderCmdBuffer, 0, 1, &viewport);
				vkCmdSetScissor(currentRenderCmdBuffer, 0, 1, &scissor);
			}

			pipeline->Bind(currentRenderCmdBuffer);

			VkBuffer vertexBuffers[]{ vertexBuffer->GetNative() };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(currentRenderCmdBuffer, 0, 1, vertexBuffers, offsets);
			vkCmdBindIndexBuffer(currentRenderCmdBuffer, indexBuffer->GetNative(), 0, VK_INDEX_TYPE_UINT32);

			auto sets = shader->GetCurrentDescriptorSet();

			vkCmdBindDescriptorSets(currentRenderCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
				pipeline->GetPipelineLayout(), 0, 1, &sets, 0, nullptr);

			vkCmdDrawIndexed(currentRenderCmdBuffer, count, 1, 0, 0, 0);

			// RenderPass End
			// RenderPass End
			// RenderPass End
			vkCmdEndRenderPass(currentRenderCmdBuffer);

			MUR_VK_ASSERT(vkEndCommandBuffer(currentRenderCmdBuffer));
		}
	}
}