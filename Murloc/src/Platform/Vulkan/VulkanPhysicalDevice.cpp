#include "murpch.hpp"

#include "VulkanPhysicalDevice.hpp"

namespace Murloc {

	namespace Utils {

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

		static VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, const Ref<VulkanContext>& context)
		{
			if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
				return capabilities.currentExtent;
			}
			else {

				auto framebufferSize = context->GetFrameBufferSize();
				VkExtent2D actualExtent = { framebufferSize.first, framebufferSize.second };

				actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
				actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

				return actualExtent;
			}
		}
	}

	VulkanPhysicalDevice::VulkanPhysicalDevice(const Ref<VulkanInstance>& instance, const Ref<VulkanContext>& context)
		: m_VulkanContext(context)
	{
		uint32_t deviceCount = 0;
		MUR_VK_ASSERT(vkEnumeratePhysicalDevices(instance->GetNative(), &deviceCount, nullptr));
		MUR_CORE_ASSERT(deviceCount, "Failed to find GPUs with Vulkan support!");

		std::vector<VkPhysicalDevice> devices(deviceCount);
		MUR_VK_ASSERT(vkEnumeratePhysicalDevices(instance->GetNative(), &deviceCount, devices.data()));

		for (const auto& device : devices) {
			if (IsDeviceSuitable(device)) {
				m_PhysicalDevice = device;
				break;
			}
		}
		MUR_CORE_ASSERT(m_PhysicalDevice, "Failed to find suitable GPU!");

		PopulateSwapchainSupportDetails();
	}

	void VulkanPhysicalDevice::PopulateSwapchainSupportDetails()
	{
		VkSurfaceKHR surface = m_VulkanContext->GetSurface();
		
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;

		// Query for swap chain capabilities
		MUR_VK_ASSERT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_PhysicalDevice, surface, &capabilities));

		// Query for swap chain available formats
		uint32_t formatCount;
		MUR_VK_ASSERT(vkGetPhysicalDeviceSurfaceFormatsKHR(m_PhysicalDevice, surface, &formatCount, nullptr));

		if (formatCount != 0) {
			formats.resize(formatCount);
			MUR_VK_ASSERT(vkGetPhysicalDeviceSurfaceFormatsKHR(m_PhysicalDevice, surface, &formatCount, formats.data()));
		}
		// Query for swap chain available present modes
		uint32_t presentModeCount;
		MUR_VK_ASSERT(vkGetPhysicalDeviceSurfacePresentModesKHR(m_PhysicalDevice, surface, &presentModeCount, nullptr));

		if (presentModeCount != 0) {
			presentModes.resize(presentModeCount);
			MUR_VK_ASSERT(vkGetPhysicalDeviceSurfacePresentModesKHR(m_PhysicalDevice, surface, &presentModeCount, presentModes.data()));
		}

		m_SupportDetails.SurfaceFormat = Utils::ChooseSwapSurfaceFormat(formats);
		m_SupportDetails.PresentMode = Utils::ChooseSwapPresentMode(presentModes);
		m_SupportDetails.Extent = Utils::ChooseSwapExtent(capabilities, m_VulkanContext);
		m_SupportDetails.Capabilities = capabilities;

		m_SupportDetails.MinImageCount = capabilities.minImageCount + 1;
		if (capabilities.maxImageCount > 0 && m_SupportDetails.MinImageCount > capabilities.maxImageCount) { // ?
			m_SupportDetails.MinImageCount = capabilities.maxImageCount;
		}
	}

	// Can specify what type of gpu is best
	bool VulkanPhysicalDevice::IsDeviceSuitable(VkPhysicalDevice device)
	{
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto& queueFamily : queueFamilies) {
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				m_QueueFamilyIndices.GraphicsFamily = i;
			}

			VkBool32 presentSupport = false;
			MUR_VK_ASSERT(vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_VulkanContext->GetSurface(), &presentSupport));

			if(presentSupport)
				m_QueueFamilyIndices.PresentFamily = i;

			if (m_QueueFamilyIndices.IsComplete())
				break;

			i++;
		}

		return m_QueueFamilyIndices.IsComplete();
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

		return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
			deviceFeatures.geometryShader;
	}

}