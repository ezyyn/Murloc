#pragma once

#include <vulkan/vulkan.h>

namespace Murloc {

	struct QueueFamilyIndices {
		std::optional<uint32_t> GraphicsFamily;
		std::optional<uint32_t> PresentFamily;

		bool IsComplete() {
			return GraphicsFamily.has_value() && PresentFamily.has_value();
		}
	};

	struct SwapchainSupportDetails {
		uint32_t MinImageCount;
		VkSurfaceFormatKHR SurfaceFormat;
		VkPresentModeKHR PresentMode;
		VkSurfaceCapabilitiesKHR Capabilities;
		VkExtent2D Extent;
	};

	class VulkanPhysicalDevice {
	public:
		VulkanPhysicalDevice();

		VkPhysicalDevice GetNative() const { return m_PhysicalDevice; };

		const QueueFamilyIndices& GetQueueFamilyIndices() const { return m_QueueFamilyIndices; }

		const SwapchainSupportDetails& GetSupportDetails() { return m_SupportDetails; }
	private:
		void PopulateSwapchainSupportDetails();

		bool IsDeviceSuitable(VkPhysicalDevice device);
		
		QueueFamilyIndices m_QueueFamilyIndices;
		SwapchainSupportDetails m_SupportDetails;

		VkPhysicalDevice m_PhysicalDevice{ VK_NULL_HANDLE };
	};

}