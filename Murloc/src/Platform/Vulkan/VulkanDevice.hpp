#pragma once

#include "Platform/Vulkan/VulkanPhysicalDevice.hpp"

#include <vulkan/vulkan.h>

namespace Murloc {

	class VulkanDevice {
	public:
		VulkanDevice();
		~VulkanDevice();

		VkDevice GetNative() const { return m_Device; };

		const Ref<VulkanPhysicalDevice>& GetPhysicalDevice() const { return m_PhysicalDevice; }

		VkQueue GetGraphicsQueue() const { return m_GraphicsQueue; }
		VkQueue GetPresentQueue() const { return m_PresentQueue; }
	private:
		VkQueue m_GraphicsQueue{ VK_NULL_HANDLE };
		VkQueue m_PresentQueue{ VK_NULL_HANDLE };

		VkDevice m_Device{ VK_NULL_HANDLE };
		Ref<VulkanPhysicalDevice> m_PhysicalDevice;
	};
}