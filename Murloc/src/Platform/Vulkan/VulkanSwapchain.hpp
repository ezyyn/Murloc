#pragma once

#include "Platform/Vulkan/Vulkan.hpp"
#include "Platform/Vulkan/VulkanDevice.hpp"
#include "Platform/Vulkan/VulkanContext.hpp"

namespace Murloc {

	class VulkanSwapchain {
	public:
		VulkanSwapchain(const Ref<VulkanDevice>& device, const Ref<VulkanContext>& context);
		~VulkanSwapchain();

		void OnResize(uint32_t width, uint32_t height);

	private:
		void Create(uint32_t width, uint32_t height);

		VkSurfaceKHR m_Surface;
		VkSwapchainKHR m_Swapchain;

		SwapchainSupportDetails m_SwapchainSupportDetails;

		Ref<VulkanDevice> m_Device;
	};

}