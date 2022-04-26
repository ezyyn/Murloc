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
#include "VulkanFramebuffer.hpp"

#include "Murloc/Renderer/BufferLayout.hpp"

namespace Murloc {

	struct RenderObjects {

		Ref<VulkanSwapchain> Swapchain;
		Ref<VulkanShader> Shader;
		Ref<VulkanRenderPass> RenderPass;
		Ref<VulkanPipeline> GraphicsPipeline;
		Ref<VulkanFramebuffer> Framebuffer;
		Ref<VulkanCommandBuffers> CommandBuffers;

		~RenderObjects() {
			CommandBuffers.reset();
			Framebuffer.reset();
			GraphicsPipeline.reset();
			RenderPass.reset();
			Shader.reset();
			Swapchain.reset();
		}

	};

	static RenderObjects* s_Objects{ nullptr };

	static std::vector<VkSemaphore> imageAvailableSemaphores;
	static std::vector<VkSemaphore> renderFinishedSemaphores;
	static std::vector<VkFence> inFlightFences;

	static int MAX_FRAMES_IN_FLIGHT = 2;

	void VulkanRenderer::Init()
	{
		s_Objects = new RenderObjects;

		Vulkan::Init();

		s_Objects->Swapchain = CreateRef<VulkanSwapchain>();

		s_Objects->Shader = CreateRef<VulkanShader>("assets/shaders/FlatColor.glsl");

		s_Objects->RenderPass = CreateRef<VulkanRenderPass>(s_Objects->Swapchain);

		s_Objects->CommandBuffers = CreateRef<VulkanCommandBuffers>(MAX_FRAMES_IN_FLIGHT);

		s_Objects->Framebuffer = CreateRef<VulkanFramebuffer>(s_Objects->RenderPass, s_Objects->Swapchain);

		BufferLayout quadBufferLayout = {
			{ShaderDataType::Float3, "a_Position"},
			{ShaderDataType::Float4, "a_Color"}
		};

		s_Objects->GraphicsPipeline = CreateRef<VulkanPipeline>(s_Objects->Shader, s_Objects->RenderPass, s_Objects->Swapchain, quadBufferLayout);

		imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
		{
			MUR_VK_ASSERT(vkCreateSemaphore(Vulkan::GetDevice()->GetNative(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]));
			MUR_VK_ASSERT(vkCreateSemaphore(Vulkan::GetDevice()->GetNative(), &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]));
			MUR_VK_ASSERT(vkCreateFence(Vulkan::GetDevice()->GetNative(), &fenceInfo, nullptr, &inFlightFences[i]));
		}
	}

	void VulkanRenderer::Shutdown()
	{
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(Vulkan::GetDevice()->GetNative(), renderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(Vulkan::GetDevice()->GetNative(), imageAvailableSemaphores[i], nullptr);
			vkDestroyFence(Vulkan::GetDevice()->GetNative(), inFlightFences[i], nullptr);
		}
		delete s_Objects;
		Vulkan::Shutdown();
	}

	void VulkanRenderer::OnResize(uint32_t width, uint32_t height)
	{
		vkDeviceWaitIdle(Vulkan::GetDevice()->GetNative());

		// Clean up order matters
		s_Objects->Framebuffer->Cleanup();
		s_Objects->GraphicsPipeline->Cleanup();
		s_Objects->RenderPass->Cleanup();
		s_Objects->Swapchain->Cleanup();

		// Recreate SwapChain
		// Recreate ImageViews
		s_Objects->Swapchain->Recreate(width, height);
		// Recreate RenderPass
		s_Objects->RenderPass->Recreate();
		// Recreate GraphicsPipeline

		s_Objects->GraphicsPipeline->Recreate(s_Objects->RenderPass, s_Objects->Swapchain);
		// Recreate Framebuffers
		s_Objects->Framebuffer->RecreateFramebuffers(s_Objects->RenderPass, s_Objects->Swapchain);
	}

	void VulkanRenderer::WaitIdle()
	{
		vkDeviceWaitIdle(Vulkan::GetDevice()->GetNative());
	}

	void VulkanRenderer::Begin()
	{
		// Push constants,...
	}

	static uint32_t s_CurrentFrame{ 0 };

	void VulkanRenderer::Render(const Ref<VulkanVertexBuffer>& vertexBuffer, uint32_t count)
	{
		// Flush

		vkWaitForFences(Vulkan::GetDevice()->GetNative(), 1, &inFlightFences[s_CurrentFrame], VK_TRUE, UINT64_MAX);

		uint32_t s_ImageIndex;
		MUR_VK_ASSERT(vkAcquireNextImageKHR(Vulkan::GetDevice()->GetNative(), s_Objects->Swapchain->GetNative(), 
			UINT64_MAX, imageAvailableSemaphores[s_CurrentFrame], VK_NULL_HANDLE, &s_ImageIndex));

		vkResetFences(Vulkan::GetDevice()->GetNative(), 1, &inFlightFences[s_CurrentFrame]);

		{
			s_Objects->CommandBuffers->Begin(s_CurrentFrame);
			{
				s_Objects->RenderPass->Begin(
					s_Objects->Framebuffer->GetSwapchainFramebuffers()[s_ImageIndex], 
					s_Objects->CommandBuffers->GetNative(s_CurrentFrame));

				s_Objects->GraphicsPipeline->Bind(s_Objects->CommandBuffers->GetNative(s_CurrentFrame));

				VkBuffer vertexBuffers[]{ vertexBuffer->GetNative() };
				VkDeviceSize offsets[] = { 0 };
				vkCmdBindVertexBuffers(s_Objects->CommandBuffers->GetNative(s_CurrentFrame), 0, 1, vertexBuffers, offsets);

				vkCmdDraw(s_Objects->CommandBuffers->GetNative(s_CurrentFrame), count, 1, 0, 0);

				s_Objects->RenderPass->End(s_Objects->CommandBuffers->GetNative(s_CurrentFrame));
			}
		}
		
		s_Objects->CommandBuffers->End(s_CurrentFrame);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[s_CurrentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		VkCommandBuffer cmdBuffers[] = { s_Objects->CommandBuffers->GetNative(s_CurrentFrame) };

		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = cmdBuffers;
		VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[s_CurrentFrame]};
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		MUR_VK_ASSERT(vkQueueSubmit(Vulkan::GetDevice()->GetGraphicsQueue(), 1, &submitInfo, inFlightFences[s_CurrentFrame]));

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { s_Objects->Swapchain->GetNative() };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &s_ImageIndex;
		presentInfo.pResults = nullptr; // Optional

		MUR_VK_ASSERT(vkQueuePresentKHR(Vulkan::GetDevice()->GetPresentQueue(), &presentInfo));

		s_CurrentFrame = (s_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

}