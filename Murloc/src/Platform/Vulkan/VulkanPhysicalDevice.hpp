#pragma once

#include "Platform/Vulkan/Vulkan.hpp"

#include "Platform/Vulkan/VulkanContext.hpp"

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
		VulkanPhysicalDevice(const Ref<VulkanInstance>& instance, const Ref<VulkanContext>& context);

		VkPhysicalDevice GetNative() const { return m_PhysicalDevice; };

		const QueueFamilyIndices& GetQueueFamilyIndices() const { return m_QueueFamilyIndices; }

		const SwapchainSupportDetails& GetSupportDetails() { return m_SupportDetails; }
	private:
		void PopulateSwapchainSupportDetails();

		bool IsDeviceSuitable(VkPhysicalDevice device);
		
		QueueFamilyIndices m_QueueFamilyIndices;
		SwapchainSupportDetails m_SupportDetails;

		Ref<VulkanContext> m_VulkanContext;

		VkPhysicalDevice m_PhysicalDevice{ VK_NULL_HANDLE };
	};

}