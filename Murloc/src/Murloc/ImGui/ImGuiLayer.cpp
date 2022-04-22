#include "murpch.hpp"

#include "ImGuiLayer.hpp"

#pragma warning(push, 0)
#include <imgui.h>
#pragma warning(pop)

#pragma warning(push, 0)
#include <backends/imgui_impl_vulkan.h>
#include <backends/imgui_impl_glfw.h>
#pragma warning(pop)

#include "Murloc/Core/Application.hpp"

namespace Murloc {

	void ImGuiLayer::OnAttach()
	{
		IMGUI_CHECKVERSION();

		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO(); 
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;			// Enable Multi-Viewport / Platform Windows

		ImGui::StyleColorsDark();

		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get()->GetWindow()->GetNativeWindow());

		ImGui_ImplGlfw_InitForVulkan(window, true);

/*
		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance = g_Instance;
		init_info.PhysicalDevice = g_PhysicalDevice;
		init_info.Device = g_Device;
		init_info.QueueFamily = g_QueueFamily;
		init_info.Queue = g_Queue;
		init_info.PipelineCache = g_PipelineCache;
		init_info.DescriptorPool = g_DescriptorPool;
		init_info.Subpass = 0;
		init_info.MinImageCount = g_MinImageCount;
		init_info.ImageCount = wd->ImageCount;
		init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		init_info.Allocator = g_Allocator;
		init_info.CheckVkResultFn = check_vk_result;*/

		//ImGui_ImplVulkan_Init()
	}

	void ImGuiLayer::OnDetach()
	{
	}

	void ImGuiLayer::OnUpdate(Timestep& ts)
	{
	}

	void ImGuiLayer::OnImGuiRender()
	{
	}

	void ImGuiLayer::OnEvent(Event& e)
	{
	}
}