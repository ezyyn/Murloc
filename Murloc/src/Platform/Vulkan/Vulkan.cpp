#include "murpch.hpp"

#include "Vulkan.hpp"

#include "VulkanPhysicalDevice.hpp"
#include "VulkanUtils.hpp"

#include "Murloc/Core/Application.hpp"

#include <glfw/glfw3.h>

namespace Murloc {

	struct VulkanData {
		
		Ref<VulkanDevice> Device;

		VkSurfaceKHR Surface{ VK_NULL_HANDLE };
		VkInstance Instance{ VK_NULL_HANDLE };
		VkDebugUtilsMessengerEXT DebugMessenger{ VK_NULL_HANDLE };
	};

	static bool s_ValidationLayersEnabled;

	static VulkanData* s_Objects{ nullptr };

	void Vulkan::Init(bool validationLayers)
	{
		s_Objects = new VulkanData;
		s_ValidationLayersEnabled = validationLayers;

		CreateInstance();
		CreateDebugger();

		MUR_VK_ASSERT(glfwCreateWindowSurface(
			s_Objects->Instance, 
			static_cast<GLFWwindow*>(Application::Get()->GetWindow()->GetNativeWindow()), 
			nullptr, 
			&s_Objects->Surface));

		s_Objects->Device = CreateRef<VulkanDevice>();
	}

	void Vulkan::Shutdown()
	{
		s_Objects->Device.reset();

		// Debugger destruction
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(s_Objects->Instance, "vkDestroyDebugUtilsMessengerEXT");
		MUR_CORE_ASSERT(func);
		func(s_Objects->Instance, s_Objects->DebugMessenger, nullptr);

		vkDestroySurfaceKHR(s_Objects->Instance, s_Objects->Surface, nullptr);
		vkDestroyInstance(s_Objects->Instance, nullptr);

		delete s_Objects;
	}

	VkInstance Vulkan::GetVulkanInstance()
	{
		return s_Objects->Instance;
	}

	VkSurfaceKHR Vulkan::GetSurface()
	{
		return s_Objects->Surface;
	}

	const Ref<VulkanDevice>& Vulkan::GetDevice()
	{
		return s_Objects->Device;
	}

	bool Vulkan::ValidationLayersEnabled()
	{
		return s_ValidationLayersEnabled;
	}

	std::pair<uint32_t, uint32_t> Vulkan::GetFramebufferSize()
	{
		int width, height;
		glfwGetFramebufferSize(static_cast<GLFWwindow*>(Application::Get()->GetWindow()->GetNativeWindow()), &width, &height);
		return { (uint32_t)width,(uint32_t)height };
	}

	void Vulkan::CreateInstance()
	{
		// App info
		Application* app = Application::Get();
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = app->GetSpecification().Title.c_str();
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "Murloc Engine";
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
			validationLayers = { "VK_LAYER_KHRONOS_validation" };
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
		MUR_VK_ASSERT(vkCreateInstance(&createInfo, nullptr, &s_Objects->Instance));
		MUR_CORE_INFO("Successfully created VkInstance!");
	}

	void Vulkan::CreateDebugger()
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

		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(s_Objects->Instance, "vkCreateDebugUtilsMessengerEXT");
		MUR_CORE_ASSERT(func);
		MUR_VK_ASSERT(func(s_Objects->Instance, &createInfo, nullptr, &s_Objects->DebugMessenger));
	}

}