#include "murpch.hpp"

#include "VulkanSwapchain.hpp"

namespace Murloc {

	VulkanSwapchain::VulkanSwapchain(const Ref<VulkanDevice>& device, const Ref<VulkanContext>& context)
		: m_Device(device), m_Surface(context->GetSurface())
	{
		m_SwapchainSupportDetails = m_Device->GetPhysicalDevice()->GetSupportDetails();

		Create(m_SwapchainSupportDetails.Extent.width, m_SwapchainSupportDetails.Extent.height);
	}

	VulkanSwapchain::~VulkanSwapchain()
	{
		vkDestroySwapchainKHR(m_Device->GetNative(), m_Swapchain, nullptr);
	}

	void VulkanSwapchain::OnResize(uint32_t width, uint32_t height)
	{
		// Recreate Swapchain
		Create(width, height);
	}

	void VulkanSwapchain::Create(uint32_t width, uint32_t height)
	{
		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = m_Surface;
		createInfo.minImageCount = m_SwapchainSupportDetails.MinImageCount;
		createInfo.imageFormat = m_SwapchainSupportDetails.SurfaceFormat.format;
		createInfo.imageColorSpace = m_SwapchainSupportDetails.SurfaceFormat.colorSpace;
		createInfo.imageExtent = { width, height };
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		createInfo.preTransform = m_SwapchainSupportDetails.Capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = m_SwapchainSupportDetails.PresentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		MUR_VK_ASSERT(vkCreateSwapchainKHR(m_Device->GetNative(), &createInfo, nullptr, &m_Swapchain));

		MUR_CORE_WARN("Recreated Swapchain!");
	}

}

