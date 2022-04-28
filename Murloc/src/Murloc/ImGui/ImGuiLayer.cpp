#include "murpch.hpp"

#include "ImGuiLayer.hpp"

#pragma warning(push, 0)
#include <imgui.h>
#include <backends/imgui_impl_vulkan.h>
#include <backends/imgui_impl_glfw.h>
#pragma warning(pop)

#include "Murloc/Core/Application.hpp"

#include "Platform/Vulkan/Vulkan.hpp"
#include "Platform/Vulkan/VulkanRenderer.hpp"
#include "Platform/Vulkan/VulkanSwapchain.hpp"

namespace Murloc {

	namespace Utils {

		static void CheckVkResult(VkResult result)
		{
			MUR_VK_ASSERT(result);
		}

	}

	static std::vector<VkCommandBuffer> s_ImGuiCommandBuffers;

	void ImGuiLayer::OnAttach()
	{
		auto& device = Vulkan::GetDevice();
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
		init_info.Instance = Vulkan::GetVulkanInstance();
		init_info.PhysicalDevice = device->GetPhysicalDevice()->GetNative();
		init_info.Device = device->GetNative();
		init_info.QueueFamily = device->GetPhysicalDevice()->GetQueueFamilyIndices().GraphicsFamily.value();
		init_info.Queue = device->GetGraphicsQueue();
		init_info.PipelineCache = nullptr;
		init_info.DescriptorPool = descriptorPool;
		init_info.Subpass = 0;
		init_info.MinImageCount = 2;
		init_info.ImageCount = 2;
		init_info.Allocator = nullptr;
		init_info.CheckVkResultFn = nullptr;
		MUR_CORE_ASSERT(ImGui_ImplVulkan_Init(&init_info, VulkanRenderer::GetRenderPass()));

		io.Fonts->Build(); // ?

		// Creating secondary command buffers

		uint32_t framesInFlight = VulkanRenderer::FramesInFlight();

		VkCommandBufferAllocateInfo cmdBufAllocateInfo = {};
		cmdBufAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdBufAllocateInfo.commandPool = Vulkan::GetDevice()->GetCommandPool();
		cmdBufAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
		cmdBufAllocateInfo.commandBufferCount = framesInFlight;


		s_ImGuiCommandBuffers.resize(framesInFlight);
		vkAllocateCommandBuffers(Vulkan::GetDevice()->GetNative(), &cmdBufAllocateInfo, s_ImGuiCommandBuffers.data());
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
		FramePresent();

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
		auto device = Vulkan::GetDevice()->GetNative();

		MUR_VK_ASSERT(vkDeviceWaitIdle(device));
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void ImGuiLayer::FrameRender()
	{
		auto& swapChain = VulkanRenderer::GetSwapchain();
		auto device = Vulkan::GetDevice()->GetNative();
		// --------------------------
		VkClearValue clearValues[2];
		clearValues[0].color = { {0.1f, 0.1f,0.1f, 1.0f} };
		clearValues[1].depthStencil = { 1.0f, 0 };

		uint32_t width = 1280;
		uint32_t height = 720;

		VkCommandBufferBeginInfo drawCmdBufInfo = {};
		drawCmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		drawCmdBufInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		drawCmdBufInfo.pNext = nullptr;

		VkCommandBuffer drawCommandBuffer = VulkanRenderer::GetCurrentDrawCommandBuffer();
		{
			MUR_VK_ASSERT(vkWaitForFences(device, 1, &fd->Fence, VK_TRUE, UINT64_MAX));    // wait indefinitely instead of periodically checking

			MUR_VK_ASSERT(vkResetFences(device, 1, &fd->Fence));
		}
		{
			MUR_VK_ASSERT(vkResetCommandPool(device, Vulkan::GetDevice()->GetCommandPool(), 0));
			VkCommandBufferBeginInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
			MUR_VK_ASSERT(vkBeginCommandBuffer(fd->CommandBuffer, &info));
		}
		{
			VkRenderPassBeginInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			info.renderPass = VulkanRenderer::GetRenderPass();
			info.framebuffer = VulkanRenderer::;
			info.renderArea.extent.width = wd->Width;
			info.renderArea.extent.height = wd->Height;
			info.clearValueCount = 1;
			info.pClearValues = &wd->ClearValue;
			vkCmdBeginRenderPass(fd->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
		}

		// Record dear imgui primitives into command buffer
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), fd->CommandBuffer);

		// Submit command buffer
		vkCmdEndRenderPass(fd->CommandBuffer);
		{
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

	void ImGuiLayer::FramePresent()
	{

	}

}