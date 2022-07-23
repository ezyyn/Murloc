#include "pgpch.h"

#include "ImGuiLayer.h"

#pragma warning(push, 0)
#include <imgui.h>
#include <backends/imgui_impl_vulkan.h>
#include <backends/imgui_impl_glfw.h>
#pragma warning(pop)

#include "Pangolin/Core/Application.h"

#include "Pangolin/Renderer/Vulkan/VulkanContext.h"
#include "Pangolin/Renderer/Vulkan/Swapchain.h"
#include "Pangolin/Renderer/Vulkan/Texture.h"
#include "Pangolin/Renderer/Vulkan/Framebuffer.h"
#include "Pangolin/Renderer/Vulkan/VulkanUtils.h"

#define PG_VULKAN_FUNCTIONS
	#include "Pangolin/Renderer/RenderManager.h"

namespace PG {

	namespace Utils {

		static void CheckVkResult(VkResult r)
		{
			PG_VK_ASSERT(r);
		}

		static void SetDarkThemeV2Colors()
		{
			auto& colors = ImGui::GetStyle().Colors;
			colors[ImGuiCol_WindowBg] = ImVec4{ 0.1f, 0.105f, 0.11f, 1.0f };

			// Headers
			colors[ImGuiCol_Header] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
			colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
			colors[ImGuiCol_HeaderActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

			// Buttons
			colors[ImGuiCol_Button] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
			colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
			colors[ImGuiCol_ButtonActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

			// Frame BG
			colors[ImGuiCol_FrameBg] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
			colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
			colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

			// Tabs
			colors[ImGuiCol_Tab] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
			colors[ImGuiCol_TabHovered] = ImVec4{ 0.38f, 0.3805f, 0.381f, 1.0f };
			colors[ImGuiCol_TabActive] = ImVec4{ 0.28f, 0.2805f, 0.281f, 1.0f };
			colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
			colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };

			// Title
			colors[ImGuiCol_TitleBg] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
			colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
			colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

			// Resize Grip
			colors[ImGuiCol_ResizeGrip] = ImVec4(0.91f, 0.91f, 0.91f, 0.25f);
			colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.81f, 0.81f, 0.81f, 0.67f);
			colors[ImGuiCol_ResizeGripActive] = ImVec4(0.46f, 0.46f, 0.46f, 0.95f);

			// Scrollbar
			colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
			colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.0f);
			colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.0f);
			colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.0f);

			// Check Mark
			colors[ImGuiCol_CheckMark] = ImVec4(0.94f, 0.94f, 0.94f, 1.0f);

			// Slider
			colors[ImGuiCol_SliderGrab] = ImVec4(0.51f, 0.51f, 0.51f, 0.7f);
			colors[ImGuiCol_SliderGrabActive] = ImVec4(0.66f, 0.66f, 0.66f, 1.0f);
		}
	}

	static std::vector<VkCommandBuffer> s_ImGuiCommandBuffers;

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

		//Utils::SetDarkThemeV2Colors();


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

		uint32_t framesInFlight = RenderManager::GetSettings().FramesInFlight;
		const auto swapChain = (Swapchain*)Application::Get()->GetWindow()->GetSwapchain();

		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = 100 * IM_ARRAYSIZE(pool_sizes);
		pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
		pool_info.pPoolSizes = pool_sizes;
		PG_VK_ASSERT(vkCreateDescriptorPool(device->GetNative(), &pool_info, nullptr, &descriptorPool));

		auto& resourceFreeQueue = VulkanContext::GetContextResourceFreeQueue();

		resourceFreeQueue.PushBack(DESCRIPTOR_POOL, [device, descriptorPool]() 
			{
				vkDestroyDescriptorPool(device->GetNative(), descriptorPool, nullptr);
			});

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
		init_info.MinImageCount = framesInFlight; // Swapchain image count
		init_info.ImageCount = framesInFlight; // Swapchain image count
		init_info.Allocator = nullptr;
		init_info.CheckVkResultFn = Utils::CheckVkResult;

		PG_CORE_ASSERT(ImGui_ImplVulkan_Init(&init_info, swapChain->GetRenderPass()));
		// Upload fonts
		RenderManager::ImmediateSubmit([](VkCommandBuffer currentCommandBuffer) 
			{
				ImGui_ImplVulkan_CreateFontsTexture(currentCommandBuffer);
			});

		ImGui_ImplVulkan_DestroyFontUploadObjects();

		// Allocating secondary command buffers
		VulkanHelpers::AllocateSecondaryCommandBuffers(s_ImGuiCommandBuffers, framesInFlight);
	}

	void ImGuiLayer::Begin()
	{
		ImGui_ImplGlfw_NewFrame();
		ImGui_ImplVulkan_NewFrame();
		ImGui::NewFrame();
	}

	void ImGuiLayer::End()
	{
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
		vkDeviceWaitIdle(device);

		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void ImGuiLayer::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowResizeEvent>(PG_BIND_FN(ImGuiLayer::OnWindowResize));
	}

	bool ImGuiLayer::OnWindowResize(WindowResizeEvent& e)
	{
		return false;
	}

	void ImGuiLayer::FrameRender()
	{
		const auto swapChain = (Swapchain*)Application::Get()->GetWindow()->GetSwapchain();
		auto renderPass = swapChain->GetRenderPass();

		auto currentFrame = swapChain->GetCurrentFrame();

		RenderManager::SubmitToMain([swapChain, renderPass, currentFrame](VkCommandBuffer currentMainCommandBuffer, VkFramebuffer currentSwapchainFramebuffer)
			{
				VkClearValue clearValues[1]{};
				clearValues[0].color = { {0.0f, 0.0f,0.0f, 1.0f} };

				VkRenderPassBeginInfo renderPassBeginInfo = {};
				renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				renderPassBeginInfo.renderPass = renderPass;
				renderPassBeginInfo.renderArea.offset.x = 0;
				renderPassBeginInfo.renderArea.offset.y = 0;
				renderPassBeginInfo.renderArea.extent = swapChain->GetExtent();
				renderPassBeginInfo.clearValueCount = 1;
				renderPassBeginInfo.pClearValues = clearValues;
				renderPassBeginInfo.framebuffer = currentSwapchainFramebuffer;
				uint32_t width = swapChain->GetExtent().width;
				uint32_t height = swapChain->GetExtent().height;

				vkCmdBeginRenderPass(currentMainCommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
				{
					// ImGui secondary command buffer record
					// ImGui secondary command buffer record
					// ImGui secondary command buffer record

					// Not necessary but future multithread

					VkCommandBufferInheritanceInfo inheritanceInfo = {};
					inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
					inheritanceInfo.renderPass = swapChain->GetRenderPass();
					inheritanceInfo.framebuffer = currentSwapchainFramebuffer;

					VkCommandBufferBeginInfo cmdBufInfo = {};
					cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
					cmdBufInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
					cmdBufInfo.pInheritanceInfo = &inheritanceInfo;

					PG_VK_ASSERT(vkBeginCommandBuffer(s_ImGuiCommandBuffers[currentFrame], &cmdBufInfo));
					VkViewport viewport = {};
					viewport.x = 0.0f;
					viewport.y = 0.0f;
					viewport.width = (float)width;
					viewport.height = -(float)height;
					viewport.minDepth = 0.0f;
					viewport.maxDepth = 1.0f;
					vkCmdSetViewport(s_ImGuiCommandBuffers[currentFrame], 0, 1, &viewport);

					VkRect2D scissor = {};
					scissor.extent.width = width;
					scissor.extent.height = height;
					scissor.offset.x = 0;
					scissor.offset.y = 0;
					vkCmdSetScissor(s_ImGuiCommandBuffers[currentFrame], 0, 1, &scissor);

					// Record dear imgui primitives into command buffer
					ImDrawData* main_draw_data = ImGui::GetDrawData();
					ImGui_ImplVulkan_RenderDrawData(main_draw_data, s_ImGuiCommandBuffers[currentFrame]);
					PG_VK_ASSERT(vkEndCommandBuffer(s_ImGuiCommandBuffers[currentFrame]));
				}
				// Executing ImGui secondary command buffers
				// Executing ImGui secondary command buffers
				// Executing ImGui secondary command buffers
				std::vector<VkCommandBuffer> submitCommandBuffer;
				submitCommandBuffer.push_back(s_ImGuiCommandBuffers[currentFrame]);

				vkCmdExecuteCommands(currentMainCommandBuffer, (uint32_t)submitCommandBuffer.size(), submitCommandBuffer.data());

				vkCmdEndRenderPass(currentMainCommandBuffer);
			});
	}
}