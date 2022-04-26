#include "murpch.hpp"

#include "VulkanSwapchain.hpp"
#include "Vulkan.hpp"

namespace Murloc {

	VulkanSwapchain::VulkanSwapchain()
	{
		auto physicalDeviceSupp = Vulkan::GetDevice()->GetPhysicalDevice()->GetSupportDetails();

		m_SwapchainSupportDetails.Capabilities = physicalDeviceSupp.Capabilities;
		m_SwapchainSupportDetails.Extent = physicalDeviceSupp.Extent;
		m_SwapchainSupportDetails.MinImageCount = physicalDeviceSupp.MinImageCount;
		m_SwapchainSupportDetails.PresentMode = physicalDeviceSupp.PresentMode;
		m_SwapchainSupportDetails.SurfaceFormat = physicalDeviceSupp.SurfaceFormat;

		Recreate(m_SwapchainSupportDetails.Extent.width, m_SwapchainSupportDetails.Extent.height);
	}

	VulkanSwapchain::~VulkanSwapchain()
	{
		Cleanup();
	}

	void VulkanSwapchain::Recreate(uint32_t width, uint32_t height)
	{
		// Recreate Swapchain
		Create(width, height);
		CreateImageViews();
	}

	void VulkanSwapchain::Cleanup()
	{
		for (auto imageView : m_SwapchainImageViews) {
			vkDestroyImageView(Vulkan::GetDevice()->GetNative(), imageView, nullptr);
		}

		vkDestroySwapchainKHR(Vulkan::GetDevice()->GetNative(), m_Swapchain, nullptr);
	}

	void VulkanSwapchain::Create(uint32_t width, uint32_t height)
	{
		m_SwapChainExtent = { width, height };

		auto& indices = Vulkan::GetDevice()->GetPhysicalDevice()->GetQueueFamilyIndices();

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = Vulkan::GetSurface();
		createInfo.minImageCount = m_SwapchainSupportDetails.MinImageCount;
		createInfo.imageFormat = m_SwapchainSupportDetails.SurfaceFormat.format;
		createInfo.imageColorSpace = m_SwapchainSupportDetails.SurfaceFormat.colorSpace;
		createInfo.imageExtent = m_SwapChainExtent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		createInfo.preTransform = m_SwapchainSupportDetails.Capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = m_SwapchainSupportDetails.PresentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		uint32_t queueFamilyIndices[] = { indices.GraphicsFamily.value(), indices.PresentFamily.value() };

		if (indices.GraphicsFamily != indices.PresentFamily) {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			//createInfo.queueFamilyIndexCount = 0; // Optional
			//createInfo.pQueueFamilyIndices = nullptr; // Optional
		}

		MUR_VK_ASSERT(vkCreateSwapchainKHR(Vulkan::GetDevice()->GetNative(), &createInfo, nullptr, &m_Swapchain));

		uint32_t imageCount;
		MUR_VK_ASSERT(vkGetSwapchainImagesKHR(Vulkan::GetDevice()->GetNative(), m_Swapchain, &imageCount, nullptr));
		m_SwapchainImages.resize(imageCount);
		MUR_VK_ASSERT(vkGetSwapchainImagesKHR(Vulkan::GetDevice()->GetNative(), m_Swapchain, &imageCount, m_SwapchainImages.data()));

		m_SwapchainImageFormat = m_SwapchainSupportDetails.SurfaceFormat.format;

		MUR_CORE_WARN("Recreated Swapchain!");
	}

	void VulkanSwapchain::CreateImageViews()
	{
		m_SwapchainImageViews.resize(m_SwapchainImages.size());

		for (size_t i = 0; i < m_SwapchainImages.size(); ++i)
		{
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = m_SwapchainImages[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = m_SwapchainImageFormat;
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;
			MUR_VK_ASSERT(vkCreateImageView(Vulkan::GetDevice()->GetNative(), &createInfo, nullptr, &m_SwapchainImageViews[i]));
		}

		MUR_CORE_WARN("Recreated Image Views!");
	}

}

