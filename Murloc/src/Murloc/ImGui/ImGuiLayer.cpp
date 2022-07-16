#include "murpch.hpp"

#include "ImGuiLayer.hpp"

#pragma warning(push, 0)
#include <imgui.h>
#include <backends/imgui_impl_vulkan.h>
#include <backends/imgui_impl_glfw.h>
#pragma warning(pop)

#include "Murloc/Core/Application.hpp"

#include "Murloc/Renderer/Vulkan/VulkanContext.h"
#include "Murloc/Renderer/Vulkan/Swapchain.h"
#include "Murloc/Renderer/Renderer.hpp"

namespace Murloc {

	namespace Utils {

		static void CheckVkResult(VkResult r)
		{
			MUR_VK_ASSERT(r);
		}

	}

	static std::vector<VkCommandBuffer> s_ImGuiCommandBuffers;

	//static VkRenderPass s_RenderPasss;

	void ImGuiLayer::OnAttach()
	{
		auto device = VulkanContext::GetLogicalDevice();
		IMGUI_CHECKVERSION();

		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO(); 
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;			// Enable Multi-Viewport / Platform Windows

		io.Fonts->AddFontDefault();

		ImGui::StyleColorsDark();

		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}
		style.Colors[ImGuiCol_WindowBg] = ImVec4(0.15f, 0.15f, 0.15f, style.Colors[ImGuiCol_WindowBg].w);

		GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get()->GetWindow()->GetNativeWindow());

		// Create Descriptor Pool
		VkDescriptorPool descriptorPool;

		VkDescriptorPoolSize pool_sizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 100 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 100 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 100 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 100 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 100 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 100 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 100 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 100 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 100 }
		};

		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = 100 * IM_ARRAYSIZE(pool_sizes);
		pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
		pool_info.pPoolSizes = pool_sizes;
		MUR_VK_ASSERT(vkCreateDescriptorPool(device->GetNative(), &pool_info, nullptr, &descriptorPool));

		ImGui_ImplGlfw_InitForVulkan(window, true);

		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance = VulkanContext::GetVulkanInstance();
		init_info.PhysicalDevice = device->GetPhysicalDevice().GetNative();
		init_info.Device = device->GetNative();
		init_info.QueueFamily = device->GetPhysicalDevice().GetQueueFamilyIndices().GraphicsFamily.value();
		init_info.Queue = device->GetGraphicsQueue();
		init_info.PipelineCache = nullptr;
		init_info.DescriptorPool = descriptorPool;
		init_info.Subpass = 0;
		init_info.MinImageCount = 2; // Swapchain image count
		init_info.ImageCount = 2; // Swapchain image count
		init_info.Allocator = nullptr;
		init_info.CheckVkResultFn = Utils::CheckVkResult;
		MUR_CORE_ASSERT(ImGui_ImplVulkan_Init(&init_info, Swapchain::s_RenderPass));
		// Upload fonts
		{
			// Use any command queue
			VkCommandBuffer command_buffer = Swapchain::s_CurrentRenderCommandbuffer;

			auto device = VulkanContext::GetLogicalDevice()->GetNative();

			VkCommandBufferBeginInfo begin_info = {};
			begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
			MUR_VK_ASSERT(vkBeginCommandBuffer(command_buffer, &begin_info));

			ImGui_ImplVulkan_CreateFontsTexture(command_buffer);

			VkSubmitInfo end_info = {};
			end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			end_info.commandBufferCount = 1;
			end_info.pCommandBuffers = &command_buffer;
			MUR_VK_ASSERT(vkEndCommandBuffer(command_buffer));

			MUR_VK_ASSERT(vkQueueSubmit(VulkanContext::GetLogicalDevice()->GetGraphicsQueue(), 1, &end_info, VK_NULL_HANDLE));

			MUR_VK_ASSERT(vkDeviceWaitIdle(device));
			ImGui_ImplVulkan_DestroyFontUploadObjects();
		}

		uint32_t framesInFlight = Renderer::GetSettings().FramesInFlight;

		// Creating secondary command buffers
		VkCommandBufferAllocateInfo cmdBufAllocateInfo = {};
		cmdBufAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdBufAllocateInfo.commandPool = VulkanContext::GetLogicalDevice()->GetCommandPool();
		cmdBufAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
		cmdBufAllocateInfo.commandBufferCount = framesInFlight;

		s_ImGuiCommandBuffers.resize(framesInFlight);
		MUR_VK_ASSERT(vkAllocateCommandBuffers(VulkanContext::GetLogicalDevice()->GetNative(), &cmdBufAllocateInfo, s_ImGuiCommandBuffers.data()));
	}

	void ImGuiLayer::Begin()
	{
		ImGui_ImplGlfw_NewFrame();
		ImGui_ImplVulkan_NewFrame();
		ImGui::NewFrame();

		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;

		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

		// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
		// and handle the pass-thru hole, so we ask Begin() to not render a background.
		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
		// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
		// all active windows docked into it will lose their parent and become undocked.
		// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
		// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("DockSpace Demo", nullptr, window_flags);
		ImGui::PopStyleVar();

		ImGui::PopStyleVar(2);

		// Submit the DockSpace
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("VulkanAppDockspace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}
	}

	void ImGuiLayer::End()
	{
		//ImGui::ShowDemoWindow();

		ImGui::Begin("Settings");
		ImGui::End();

		ImGui::Begin("Viewport");
		ImGui::End();

		ImGui::End(); // Dockspace End

		ImGui::Render();

		FrameRender();

		ImGuiIO& io = ImGui::GetIO(); 
		// Update and Render additional Platform Windows
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	}

	void ImGuiLayer::OnDetach()
	{
		auto device = VulkanContext::GetLogicalDevice()->GetNative();

		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void ImGuiLayer::FrameRender()
	{
		const auto swapChain = (Swapchain*)Application::Get()->GetWindow()->GetSwapchain();

		auto device = VulkanContext::GetLogicalDevice()->GetNative();
		auto currentFramebuffer = Swapchain::s_CurrentFramebuffer;
		auto index = swapChain->s_CurrentFrame;
		auto renderPass = Swapchain::s_RenderPass;
		// --------------------------
		VkClearValue clearValues[2];
		clearValues[0].color = { {0.1f, 0.1f,0.1f, 1.0f} };
		clearValues[1].depthStencil = { 1.0f, 0 };

		uint32_t width = swapChain->GetExtent().width;
		uint32_t height = swapChain->GetExtent().height;

		VkCommandBufferBeginInfo drawCmdBufInfo = {};
		drawCmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		drawCmdBufInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		drawCmdBufInfo.pNext = nullptr;

		auto currentDrawCommandBuffer = Swapchain::s_CurrentRenderCommandbuffer;
		
		// Record ImGui command buffers
		// Record ImGui command buffers
		// Record ImGui command buffers

		{
			MUR_VK_ASSERT(vkBeginCommandBuffer(currentDrawCommandBuffer, &drawCmdBufInfo));

			VkRenderPassBeginInfo renderPassBeginInfo = {};
			renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassBeginInfo.renderPass = renderPass;
			renderPassBeginInfo.renderArea.offset.x = 0;
			renderPassBeginInfo.renderArea.offset.y = 0;
			renderPassBeginInfo.renderArea.extent = swapChain->GetExtent();
			renderPassBeginInfo.clearValueCount = 2;
			renderPassBeginInfo.pClearValues = clearValues;
			renderPassBeginInfo.framebuffer = currentFramebuffer;

			vkCmdBeginRenderPass(currentDrawCommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
			
			{
				// ImGui secondary command buffers
				// ImGui secondary command buffers
				// ImGui secondary command buffers
				VkCommandBufferInheritanceInfo inheritanceInfo = {};
				inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
				inheritanceInfo.renderPass = renderPass;
				inheritanceInfo.framebuffer = currentFramebuffer;

				VkCommandBufferBeginInfo cmdBufInfo = {};
				cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
				cmdBufInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
				cmdBufInfo.pInheritanceInfo = &inheritanceInfo;
				MUR_VK_ASSERT(vkBeginCommandBuffer(s_ImGuiCommandBuffers[index], &cmdBufInfo));

				VkViewport viewport = {};
				viewport.x = 0.0f;
				viewport.y = (float)height;
				viewport.height = -(float)height;
				viewport.width = (float)width;
				viewport.minDepth = 0.0f;
				viewport.maxDepth = 1.0f;
				vkCmdSetViewport(s_ImGuiCommandBuffers[index], 0, 1, &viewport);

				VkRect2D scissor = {};
				scissor.extent.width = width;
				scissor.extent.height = height;
				scissor.offset.x = 0;
				scissor.offset.y = 0;
				vkCmdSetScissor(s_ImGuiCommandBuffers[index], 0, 1, &scissor);

				// Record dear imgui primitives into command buffer
				ImDrawData* main_draw_data = ImGui::GetDrawData();
				ImGui_ImplVulkan_RenderDrawData(main_draw_data, s_ImGuiCommandBuffers[index]);

				MUR_VK_ASSERT(vkEndCommandBuffer(s_ImGuiCommandBuffers[index]));
				// ------------------------------------
				// ------------------------------------
				// ------------------------------------
			}
		
			std::vector<VkCommandBuffer> commandBuffers;
			commandBuffers.push_back(s_ImGuiCommandBuffers[index]);

			vkCmdExecuteCommands(currentDrawCommandBuffer, uint32_t(commandBuffers.size()), commandBuffers.data());

			vkCmdEndRenderPass(currentDrawCommandBuffer);

			MUR_VK_ASSERT(vkEndCommandBuffer(currentDrawCommandBuffer));
		}

	/*	{
			VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			VkSubmitInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			info.waitSemaphoreCount = 1;
			info.pWaitSemaphores = &image_acquired_semaphore;
			info.pWaitDstStageMask = &wait_stage;
			info.commandBufferCount = 1;
			info.pCommandBuffers = &fd->CommandBuffer;
			info.signalSemaphoreCount = 1;
			info.pSignalSemaphores = &render_complete_semaphore;

			MUR_VK_ASSERT(vkEndCommandBuffer(fd->CommandBuffer));
			MUR_VK_ASSERT(vkQueueSubmit(Vulkan::GetDevice()->GetGraphicsQueue(), 1, &info, fd->Fence));
		}*/
	}
}