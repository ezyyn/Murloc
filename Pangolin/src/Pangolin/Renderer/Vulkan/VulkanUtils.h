#pragma once

#include "Pangolin/Core/Common.h"

#include <vulkan/vulkan.h>

#include "VulkanContext.h"

namespace PG {
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

				PG_VK_ASSERT(vkCreateCommandPool(device->GetNative(), &poolInfo, nullptr, &m_Pool));
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

namespace PG::Utils {

	static uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
		auto device = VulkanContext::GetLogicalDevice();
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(device->GetPhysicalDevice().GetNative(), &memProperties);
		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		PG_CORE_ASSERT(false, "Could not find proper memory type.");
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
			//case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: PG_CORE_TRACE("Validation layer: {0}", pCallbackData->pMessage); break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:    PG_CORE_INFO("Validation layer: {0}", pCallbackData->pMessage); break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: PG_CORE_WARN("Validation layer: {0}", pCallbackData->pMessage); break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:   PG_CORE_ERROR("Validation layer: {0}", pCallbackData->pMessage); break;
			//	default:
					//PG_CORE_ASSERT(false); // Invalid severity
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

	static VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) 
	{
		auto physicalDevice = VulkanContext::GetLogicalDevice()->GetPhysicalDevice().GetNative();

		for (VkFormat format : candidates) {
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);
			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
				return format;
			}
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
				return format;
			}
		}

		PG_CORE_ASSERT(false, "Could not find supported format!");

		return VK_FORMAT_UNDEFINED;
	}

	static VkFormat FindDepthFormat() {
		return FindSupportedFormat(
			{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
		);
	}
}

namespace PG::VulkanHelpers {

	static void AllocatePrimaryCommandBuffers(std::vector<VkCommandBuffer>& commandBuffers, size_t size) 
	{
		PG_CORE_ASSERT(!commandBuffers.size(), "Command buffer must be empty!");

		auto device = VulkanContext::GetLogicalDevice()->GetNative();

		commandBuffers.resize(size);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = VulkanContext::GetLogicalDevice()->GetCommandPool();
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)size;

		PG_VK_ASSERT(vkAllocateCommandBuffers(device,
			&allocInfo, commandBuffers.data()));

		// Resource Free Queue
		// Free by command pool destruction
	}

	static void AllocateSecondaryCommandBuffers(std::vector<VkCommandBuffer>& commandBuffers, size_t size)
	{
		PG_CORE_ASSERT(!commandBuffers.size(), "Command buffer must be empty!");

		auto device = VulkanContext::GetLogicalDevice()->GetNative();

		commandBuffers.resize(size);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = VulkanContext::GetLogicalDevice()->GetCommandPool();
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
		allocInfo.commandBufferCount = (uint32_t)size;

		PG_VK_ASSERT(vkAllocateCommandBuffers(device,
			&allocInfo, commandBuffers.data()));

		// Resource Free Queue
		// Free by command pool destruction
	}

	static void CreateSemaphores(std::vector<VkSemaphore>& semaphores, size_t size, VkSemaphoreCreateFlags flags = 0)
	{
		auto device = VulkanContext::GetLogicalDevice()->GetNative();

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphoreInfo.flags = flags;

		semaphores.resize(size);

		for (size_t i = 0; i < size; ++i)
		{
			PG_VK_ASSERT(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &semaphores[i]));
		}

		// Resource Free Queue
		auto& resourceFreeQueue = VulkanContext::GetContextResourceFreeQueue();

		resourceFreeQueue.PushBack(SYNC_OBJECT, [device, size, semaphores]()
			{
				for (size_t i = 0; i < size; i++) {
					vkDestroySemaphore(device, semaphores[i], nullptr);
				}
			});
	}

	static void CreateFences(std::vector<VkFence>& fences, size_t size, VkSemaphoreCreateFlags flags = 0)
	{
		auto device = VulkanContext::GetLogicalDevice()->GetNative();

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = flags;

		fences.resize(size);

		for (size_t i = 0; i < size; ++i)
		{
			PG_VK_ASSERT(vkCreateFence(device, &fenceInfo, nullptr, &fences[i]));
		}
		// Resource Free Queue
		auto& resourceFreeQueue = VulkanContext::GetContextResourceFreeQueue();

		resourceFreeQueue.PushBack(SYNC_OBJECT, [device, size, fences]()
			{
				for (size_t i = 0; i < size; i++) {
					vkDestroyFence(device, fences[i], nullptr);
				}
			});
	}
}