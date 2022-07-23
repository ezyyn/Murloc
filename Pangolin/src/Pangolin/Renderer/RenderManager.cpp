#include "pgpch.h"

#define PG_VULKAN_FORWARD_DECLARATIONS
#include "RenderManager.h"

#include "Renderer2D.h"
#include "CommandQueue.h"
#include "Vulkan/Swapchain.h"
#include "Vulkan/VulkanUtils.h"
#include "Pangolin/Core/Application.h"

#pragma warning(push, 0)
	#include "backends/imgui_impl_vulkan.h"
#pragma warning(pop)
/****
	* Every renderer should:
	*	- Submit to the Render command buffer
	*	- Have separate VkImage, VkImageView, VkRenderPass, VkFramebuffer
	*
	* UI(ImGuiLayer) should have main framebuffer to render everything previous command buffers
	* rendered into texture.
	* Two main command buffers
	*	- 1. MainCommandBuffer - used by UI to render everything
	*	- 2. RenTexCommandBuffer -
/****/

namespace PG {

	struct DrawData {
		std::vector<VkCommandBuffer> PrimaryCommandBuffers;
		std::vector<VkCommandBuffer> SecondaryCommandBuffers;

		std::vector<VkDescriptorSet> ImGuiTextureDescriptorSets;
	};

	static CommandQueue s_OnResizeQueue;

	static DrawData s_Data;

	void RenderManager::Init()
	{
		// Creating renderers
		s_Renderers.Renderer2d = Renderer2D::Create();
	}

	void RenderManager::Shutdown()
	{
		// releasing memory depends on static inits, do not try to reset refs
	}

	// This will use imgui
	void RenderManager::SubmitToMain(CommandBufferFunc&& recordedCommandBuffer)
	{
		auto swapChain = (Swapchain*)Application::Get()->GetWindow()->GetSwapchain();

		auto currentFramebuffer = swapChain->GetCurrentFramebuffer();
		auto currentCommandBuffer = swapChain->GetCurrentMainCommandBuffer();

		// Record ImGui command buffers
		PG_VK_ASSERT(vkResetCommandBuffer(currentCommandBuffer, 0));
		{
			VkCommandBufferBeginInfo drawCmdBufInfo = {};
			drawCmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			drawCmdBufInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
			drawCmdBufInfo.pNext = nullptr;
			PG_VK_ASSERT(vkBeginCommandBuffer(currentCommandBuffer, &drawCmdBufInfo));

			// Call recorded command buffer lambda
			recordedCommandBuffer(currentCommandBuffer, currentFramebuffer);
			PG_VK_ASSERT(vkEndCommandBuffer(currentCommandBuffer));
		}
	}

	std::vector<VkCommandBuffer> RenderManager::DrawData()
	{
		PG_CORE_ASSERT(s_Data.PrimaryCommandBuffers.size() > 0, "No command buffers to submit!");

		std::vector<VkCommandBuffer> drawData = s_Data.PrimaryCommandBuffers;

		s_Data.PrimaryCommandBuffers.clear();

		return drawData;
	}

	void RenderManager::SubmitPrimary(const std::function<VkCommandBuffer(uint32_t)>& func)
	{
		auto swapChain = (Swapchain*)Application::Get()->GetWindow()->GetSwapchain();

		uint32_t currentFrame = swapChain->GetCurrentFrame();

		VkCommandBuffer commandBuffersToSubmit = func(currentFrame);

		for (size_t i = 0; i < s_Data.PrimaryCommandBuffers.size(); ++i)
		{
			if (commandBuffersToSubmit == s_Data.PrimaryCommandBuffers[i])
			{
				// No need to push back cause command buffer is already in it
				// Just re-record it
				return;
			}
		}

		s_Data.PrimaryCommandBuffers.push_back(commandBuffersToSubmit);
	}

	void RenderManager::SubmitSecondary(const std::function<VkCommandBuffer(uint32_t, uint32_t)>& func)
	{
		PG_CORE_ASSERT(false);
		auto swapChain = (Swapchain*)Application::Get()->GetWindow()->GetSwapchain();

		uint32_t currentFrame = swapChain->GetCurrentFrame();
		uint32_t imageIndex = swapChain->GetCurrentImageIndex();

		VkCommandBuffer commandBuffersToSubmit = func(currentFrame, imageIndex);
		s_Data.SecondaryCommandBuffers.push_back(commandBuffersToSubmit);
	}

	void RenderManager::ImmediateSubmit(std::function<void(VkCommandBuffer)>&& recordingFunc)
	{
		auto device = VulkanContext::GetLogicalDevice()->GetNative();
		auto swapChain = (Swapchain*)Application::Get()->GetWindow()->GetSwapchain();

		auto currentCommandBuffer = swapChain->GetCurrentMainCommandBuffer();

		VkCommandBufferBeginInfo begin_info = {};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		PG_VK_ASSERT(vkBeginCommandBuffer(currentCommandBuffer, &begin_info));
		{
			recordingFunc(currentCommandBuffer);
		}
		VkSubmitInfo end_info = {};
		end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		end_info.commandBufferCount = 1;
		end_info.pCommandBuffers = &currentCommandBuffer;
		PG_VK_ASSERT(vkEndCommandBuffer(currentCommandBuffer));

		PG_VK_ASSERT(vkQueueSubmit(VulkanContext::GetLogicalDevice()->GetGraphicsQueue(), 1, &end_info, VK_NULL_HANDLE));

		// For now just wait
		PG_VK_ASSERT(vkQueueWaitIdle(VulkanContext::GetLogicalDevice()->GetGraphicsQueue()));
	}

	// FIXME: Maybe render manager should not have imgui include
	VkDescriptorSet RenderManager::AllocateImGuiTextureDescriptorSet(VkSampler sampler, VkImageView imageview, VkImageLayout imageLayout)
	{
		return ImGui_ImplVulkan_AddTexture(sampler, imageview, imageLayout);
	}

	void RenderManager::OnResize(uint32_t width, uint32_t height)
	{
		s_Renderers.Renderer2d->OnResize(width, height);
	}
}
