#include "murpch.hpp"

#include "VulkanDebug.hpp"

#include "Platform/Vulkan/Vulkan.hpp"
#include "Platform/Vulkan/VulkanUtils.hpp"

#include <vulkan/vulkan.h>

namespace Murloc {

	VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebug::DebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {

		switch (messageSeverity) {
		//case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: MUR_CORE_TRACE("Validation layer: {0}", pCallbackData->pMessage); break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:    MUR_CORE_INFO("Validation layer: {0}", pCallbackData->pMessage); break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: MUR_CORE_WARN("Validation layer: {0}", pCallbackData->pMessage); break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:   MUR_CORE_ERROR("Validation layer: {0}", pCallbackData->pMessage); break;
	//	default:
			//MUR_CORE_ASSERT(false); // Invalid severity
		}

		return VK_FALSE;
	}

	VulkanDebug::VulkanDebug(const Ref<VulkanInstance>& instance) : m_Instance(instance)
	{
		CheckValidationLayerSupport(m_Instance->GetValidationLayers());

		VkDebugUtilsMessengerCreateInfoEXT createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = VulkanDebug::DebugCallback;
		createInfo.pUserData = nullptr; // Optional

		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_Instance->GetNative(), "vkCreateDebugUtilsMessengerEXT");
		MUR_CORE_ASSERT(func);
		MUR_VK_ASSERT(func(m_Instance->GetNative(), &createInfo, nullptr, &m_Messenger));
	}

	VulkanDebug::~VulkanDebug()
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_Instance->GetNative(), "vkDestroyDebugUtilsMessengerEXT");

		MUR_CORE_ASSERT(func);

		func(m_Instance->GetNative(), m_Messenger, nullptr);
	}

	bool VulkanDebug::CheckValidationLayerSupport(const std::vector<const char*>& layers)
	{
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (auto layerName : layers) {
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers) {
				if (strcmp(layerName, layerProperties.layerName) == 0) {
					layerFound = true;
					break;
				}
			}

			if (!layerFound) {
				return false;
			}
		}

		return true;
	}
}