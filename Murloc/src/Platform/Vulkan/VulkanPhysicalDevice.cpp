#include "murpch.hpp"

#include "VulkanPhysicalDevice.hpp"

namespace Murloc {

	VulkanPhysicalDevice::VulkanPhysicalDevice(const Ref<VulkanInstance>& ainstance)
	{
		VkInstance instance = ainstance->GetNative();
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
		MUR_CORE_ASSERT(deviceCount, "Failed to find GPUs with Vulkan support!");

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

		for (const auto& device : devices) {
			if (IsDeviceSuitable(device)) {
				m_PhysicalDevice = device;
				break;
			}
		}
		MUR_CORE_ASSERT(m_PhysicalDevice, "Failed to find suitable GPU!");
	}

	// Can specify what type of gpu is best
	bool VulkanPhysicalDevice::IsDeviceSuitable(VkPhysicalDevice device)
	{
		return true;
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

		return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
			deviceFeatures.geometryShader;
	}

}