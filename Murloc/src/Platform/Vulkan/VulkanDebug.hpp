#pragma once

#include "murpch.hpp"

#include "Platform/Vulkan/Vulkan.hpp"
#include "Platform/Vulkan/VulkanInstance.hpp"

namespace Murloc {

	class VulkanDebug
	{
	public:
		VulkanDebug(const Ref<VulkanInstance>& instance);

		static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData);

		~VulkanDebug();
	private:
		bool CheckValidationLayerSupport(const std::vector<const char*>& layers);

		Ref<VulkanInstance> m_VulkanInstance;
		VkDebugUtilsMessengerEXT m_Messenger{ VK_NULL_HANDLE };
	};
}