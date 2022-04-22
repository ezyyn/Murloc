#include "murpch.hpp"

#include "VulkanInstance.hpp"

#include "Murloc/Core/Application.hpp"

#include "Platform/Vulkan/VulkanDebug.hpp"

namespace Murloc {

	VulkanInstance::VulkanInstance(std::vector<const char*>& extensions)
	{
		// App info
		Application* app = Application::Get();
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = app->GetSpecification().Title.c_str();
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "Murloc Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		// Vulkan Instance
		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};

		if (Vulkan::ValidationLayerEnabled()) {

			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

			// Validation layers
			m_ValidationLayers = { "VK_LAYER_KHRONOS_validation" };
			createInfo.enabledLayerCount = static_cast<uint32_t>(m_ValidationLayers.size());
			createInfo.ppEnabledLayerNames = m_ValidationLayers.data();

			{
				// For vkCreateInstance and vkDestroyInstance debug
				debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
				debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
				debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
				debugCreateInfo.pfnUserCallback = VulkanDebug::DebugCallback; // Good enough for now
			}

			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
		}

		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();
		MUR_VK_ASSERT(vkCreateInstance(&createInfo, nullptr, &m_VulkanInstance));
		MUR_CORE_INFO("Successfully created VkInstance!");
	}

	VulkanInstance::~VulkanInstance()
	{
		vkDestroyInstance(m_VulkanInstance, nullptr);
	}

}