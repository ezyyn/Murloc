#pragma once

#include "Vulkan.hpp"
#include "Platform/Vulkan/VulkanPhysicalDevice.hpp"

namespace Murloc {

	class VulkanDevice {
	public:
		VulkanDevice(const Ref<VulkanInstance>& instance, const Ref<VulkanContext>& context);
		~VulkanDevice();

		VkDevice GetNative() const { return m_Device; };

		const Ref<VulkanPhysicalDevice>& GetPhysicalDevice() const { return m_PhysicalDevice; }
	private:
		VkQueue m_GraphicsQueue{ VK_NULL_HANDLE };
		VkQueue m_PresentQueue{ VK_NULL_HANDLE };

		VkDevice m_Device{ VK_NULL_HANDLE };
		Ref<VulkanPhysicalDevice> m_PhysicalDevice;
	};
}