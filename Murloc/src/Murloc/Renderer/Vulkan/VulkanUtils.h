#pragma once

#include "Murloc/Core/Common.hpp"

#include <vulkan/vulkan.h>

namespace Murloc {
	// Temp command pool for short-lived buffers
	class ScopeCommandPool {
	public:
		ScopeCommandPool() {
			VkCommandPool tempPool{ VK_NULL_HANDLE };
			{
				auto device = VulkanContext::GetLogicalDevice();

				VkCommandPoolCreateInfo poolInfo{};
				poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
				poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
				poolInfo.queueFamilyIndex = device->GetPhysicalDevice().GetQueueFamilyIndices().GraphicsFamily.value();

				MUR_VK_ASSERT(vkCreateCommandPool(device->GetNative(), &poolInfo, nullptr, &m_Pool));
			}
		}

		VkCommandPool GetNative() const { return m_Pool; }

		~ScopeCommandPool() 
		{
			vkDestroyCommandPool(VulkanContext::GetLogicalDevice()->GetNative(), m_Pool, nullptr);
		}
	private:
		VkCommandPool m_Pool;
	};
}

namespace Murloc::Utils {

	static uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
		auto device = VulkanContext::GetLogicalDevice();
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(device->GetPhysicalDevice().GetNative(), &memProperties);
		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		MUR_CORE_ASSERT(false, "Could not find proper memory type.");
		return 0;
	}

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

	static VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		for (const auto& availableFormat : availableFormats) {
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				return availableFormat;
			}
		}

		return availableFormats[0];
	}

	static VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
	{
		for (const auto& availablePresentMode : availablePresentModes) {
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				return availablePresentMode;
			}
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}
}