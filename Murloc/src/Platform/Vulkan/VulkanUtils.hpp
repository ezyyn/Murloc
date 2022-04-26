#pragma once

#include "Murloc/Core/Common.hpp"

#include <vulkan/vulkan.h>

namespace Murloc::Utils {

	static bool CheckValidationLayerSupport(const std::vector<const char*>& layers)
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


	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
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

}