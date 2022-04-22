#pragma once

#include "Platform/Vulkan/Vulkan.hpp"

#include "Platform/Vulkan/VulkanInstance.hpp"

namespace Murloc {

	class VulkanPhysicalDevice {
	public:
		VulkanPhysicalDevice(const Ref<VulkanInstance>& instance);

	private:
		bool IsDeviceSuitable(VkPhysicalDevice device);

		VkPhysicalDevice m_PhysicalDevice{ VK_NULL_HANDLE };
	};

}