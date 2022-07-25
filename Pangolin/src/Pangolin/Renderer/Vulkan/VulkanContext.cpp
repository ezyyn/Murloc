#include "pgpch.h"

#include "VulkanContext.h"

#include "Pangolin/Core/Application.h"

#include "Pangolin/Renderer/Vulkan/VulkanUtils.h"

#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>

namespace PG {

	struct VulkanHandle {
		Ref<LogicalDevice> LgclDevice;

		VkSurfaceKHR Surface{ VK_NULL_HANDLE };
		VkInstance Instance{ VK_NULL_HANDLE };
		VkDebugUtilsMessengerEXT DebugMessenger{ VK_NULL_HANDLE };
	};

	static VulkanHandle s_Handles;

	static CommandQueue s_ContextResourceFreeQueue;

	static bool s_ValidationLayersEnabled = true;

	void VulkanContext::Init()
	{
		CreateInstance();
		CreateDebugger();
		CreateSurface();

		s_Handles.LgclDevice = CreateRef<LogicalDevice>();
	}

	void VulkanContext::Shutdown()
	{
		// Wait for the GPU to finish its work
		vkDeviceWaitIdle(s_Handles.LgclDevice->GetNative());

		s_ContextResourceFreeQueue.Execute();
	}

	VkInstance VulkanContext::GetVulkanInstance()
	{
		return s_Handles.Instance;
	}

	VkSurfaceKHR VulkanContext::GetSurface()
	{
		return s_Handles.Surface;
	}

	LogicalDevice* VulkanContext::GetLogicalDevice()
	{
		return s_Handles.LgclDevice.get();
	}

	bool VulkanContext::ValidationLayersEnabled()
	{
		return s_ValidationLayersEnabled;
	}

	CommandQueue& VulkanContext::GetContextResourceFreeQueue()
	{
		return s_ContextResourceFreeQueue;
	}

	void VulkanContext::CreateInstance()
	{
		// App info
		Application* app = Application::Get();
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = app->GetSpecification().Title.c_str();
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "Pangolin Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_2;

		// Vulkan Instance
		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};

		std::vector<const char*> extensions;
		extensions.push_back("VK_KHR_surface");
		extensions.push_back("VK_KHR_win32_surface");

		std::vector<const char*> validationLayers;

		if (s_ValidationLayersEnabled) {

			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			extensions.push_back("VK_EXT_debug_report");

			// Validation layers
			validationLayers = { 
				"VK_LAYER_KHRONOS_validation",
			};
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();

			{
				// For vkCreateInstance and vkDestroyInstance debug
				debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
				debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
				debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
				debugCreateInfo.pfnUserCallback = Utils::DebugCallback;
			}

			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
		}

		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();
		PG_VK_ASSERT(vkCreateInstance(&createInfo, nullptr, &s_Handles.Instance));
		PG_CORE_INFO("Successfully created VkInstance!");

		s_ContextResourceFreeQueue.PushBack(INSTANCE, []()
			{
				vkDestroyInstance(s_Handles.Instance, nullptr);
			});
	}

	void VulkanContext::CreateDebugger()
	{
		std::vector<const char*> validationLayers = {
			"VK_LAYER_KHRONOS_validation",
			"VK_LAYER_LUNARG_standard_validation"
		};
		Utils::CheckValidationLayerSupport(validationLayers);

		VkDebugUtilsMessengerCreateInfoEXT createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = Utils::DebugCallback;
		createInfo.pUserData = nullptr; // Optional

		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(s_Handles.Instance, "vkCreateDebugUtilsMessengerEXT");
		PG_CORE_ASSERT(func);
		PG_VK_ASSERT(func(s_Handles.Instance, &createInfo, nullptr, &s_Handles.DebugMessenger));

		s_ContextResourceFreeQueue.PushBack(DEBUG_MESSENGER,
			[]()
			{
				// Debugger destruction
				auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(s_Handles.Instance, "vkDestroyDebugUtilsMessengerEXT");
				PG_CORE_ASSERT(func);
				func(s_Handles.Instance, s_Handles.DebugMessenger, nullptr);
			});
	}

	void VulkanContext::CreateSurface()
	{
		auto window = static_cast<GLFWwindow*>(Application::Get()->GetWindow()->GetNativeWindow());

		PG_VK_ASSERT(glfwCreateWindowSurface(
			s_Handles.Instance, window,
			nullptr,
			&s_Handles.Surface));

		s_ContextResourceFreeQueue.PushBack(SURFACE,
			[]()
			{
				vkDestroySurfaceKHR(s_Handles.Instance, s_Handles.Surface, nullptr);
			});
	}

}