#pragma once

#include <vulkan/vulkan.h>

namespace Murloc {

	class VulkanSwapchain {
	public:
		VulkanSwapchain();
		~VulkanSwapchain();

		void Recreate(uint32_t width, uint32_t height);

		const std::vector<VkImageView>& GetImageViews() const { return m_SwapchainImageViews; }

		VkExtent2D GetSwapChainExtent() const { return m_SwapChainExtent; }

		VkFormat GetSwapchainImageFormat() const {
			return m_SwapchainImageFormat;
		}

		VkSwapchainKHR GetNative() const { return m_Swapchain; };

		void Cleanup();
	private:

		void Create(uint32_t width, uint32_t height);
		void CreateImageViews();

		VkFormat m_SwapchainImageFormat;
		VkSwapchainKHR m_Swapchain{ VK_NULL_HANDLE };

		VkExtent2D m_SwapChainExtent;

		struct SupportDetails {
			uint32_t MinImageCount;
			VkSurfaceFormatKHR SurfaceFormat;
			VkPresentModeKHR PresentMode;
			VkSurfaceCapabilitiesKHR Capabilities;
			VkExtent2D Extent;
		};

		SupportDetails m_SwapchainSupportDetails;

		std::vector<VkImage> m_SwapchainImages;
		std::vector<VkImageView> m_SwapchainImageViews;
	};

}