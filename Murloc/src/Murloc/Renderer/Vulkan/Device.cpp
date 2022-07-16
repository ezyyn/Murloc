#include "murpch.hpp"
#include "Device.h"

#include "VulkanContext.h"

#include "VulkanUtils.h"

namespace Murloc {

	// Logical device Logical device Logical device Logical device Logical device Logical device
	// Logical device Logical device Logical device Logical device Logical device Logical device
	// Logical device Logical device Logical device Logical device Logical device Logical device

	LogicalDevice::LogicalDevice()
	{
		const auto& indices = m_PhysicalDevice.GetQueueFamilyIndices();

		// Manage queues
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

		std::set<uint32_t> uniqueQueueFamilies = { indices.GraphicsFamily.value(), indices.PresentFamily.value() };

		float queuePriority = 1.0f;
		for (auto& queueFamily : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures{};
		// Manage device
		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.pEnabledFeatures = &deviceFeatures;

		createInfo.enabledExtensionCount = 0;
		createInfo.enabledLayerCount = 0;

		const std::vector<const char*> extensions{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		std::vector<const char*> validationLayers{
			"VK_LAYER_KHRONOS_validation"
		};

		if (VulkanContext::ValidationLayersEnabled()) {
			// Extension layers

			createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
			createInfo.ppEnabledExtensionNames = extensions.data();

			// Validation layers
			validationLayers.push_back("VK_LAYER_KHRONOS_validation");

			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}

		MUR_VK_ASSERT(vkCreateDevice(m_PhysicalDevice.GetNative(), &createInfo, nullptr, &m_NativeDevice));

		vkGetDeviceQueue(m_NativeDevice, indices.GraphicsFamily.value(), 0, &m_GraphicsQueue);
		vkGetDeviceQueue(m_NativeDevice, indices.PresentFamily.value(), 0, &m_PresentQueue);

		// Command pool
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = m_PhysicalDevice.GetQueueFamilyIndices().GraphicsFamily.value();

		MUR_VK_ASSERT(vkCreateCommandPool(m_NativeDevice, &poolInfo, nullptr, &m_CommandPool));

		auto& freeQueue = VulkanContext::GetContextResourceFreeQueue();

		freeQueue.PushBack(COMMAND_POOLS, [m_NativeDevice = m_NativeDevice, m_CommandPool = m_CommandPool]()
			{
				vkDestroyCommandPool(m_NativeDevice, m_CommandPool, nullptr);
			});

		freeQueue.PushBack(LOGICAL_DEVICE, [m_NativeDevice = m_NativeDevice]()
			{
				vkDestroyDevice(m_NativeDevice, nullptr);
			});
	}

	LogicalDevice::~LogicalDevice()
	{
	}

	// Physical device Physical device Physical device Physical device Physical device Physical device
	// Physical device Physical device Physical device Physical device Physical device Physical device
	// Physical device Physical device Physical device Physical device Physical device Physical device


	PhysicalDevice::PhysicalDevice()
	{
		auto instance = VulkanContext::GetVulkanInstance();

		uint32_t deviceCount = 0;
		MUR_VK_ASSERT(vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr));
		MUR_CORE_ASSERT(deviceCount, "Failed to find GPUs with Vulkan support!");

		std::vector<VkPhysicalDevice> devices(deviceCount);
		MUR_VK_ASSERT(vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data()));

		for (const auto& device : devices) {
			if (IsDeviceSuitable(device)) {
				m_NativePhysicalDevice = device;
				break;
			}
		}
		MUR_CORE_ASSERT(m_NativePhysicalDevice, "Failed to find suitable GPU!");

		PopulateSwapchainSupportDetails();
	}

	PhysicalDevice::~PhysicalDevice()
	{
	}

	void PhysicalDevice::PopulateSwapchainSupportDetails()
	{
		VkSurfaceKHR surface = VulkanContext::GetSurface();

		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;

		// Query for swap chain capabilities
		MUR_VK_ASSERT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_NativePhysicalDevice, surface, &capabilities));

		// Query for swap chain available formats
		uint32_t formatCount;
		MUR_VK_ASSERT(vkGetPhysicalDeviceSurfaceFormatsKHR(m_NativePhysicalDevice, surface, &formatCount, nullptr));

		if (formatCount != 0) {
			formats.resize(formatCount);
			MUR_VK_ASSERT(vkGetPhysicalDeviceSurfaceFormatsKHR(m_NativePhysicalDevice, surface, &formatCount, formats.data()));
		}
		// Query for swap chain available present modes
		uint32_t presentModeCount;
		MUR_VK_ASSERT(vkGetPhysicalDeviceSurfacePresentModesKHR(m_NativePhysicalDevice, surface, &presentModeCount, nullptr));

		if (presentModeCount != 0) {
			presentModes.resize(presentModeCount);
			MUR_VK_ASSERT(vkGetPhysicalDeviceSurfacePresentModesKHR(m_NativePhysicalDevice, surface, &presentModeCount, presentModes.data()));
		}

		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(m_NativePhysicalDevice, &properties);
		m_SupportDetails.Properties = properties;

		m_SupportDetails.SurfaceFormat = Utils::ChooseSwapSurfaceFormat(formats);
		m_SupportDetails.PresentMode = Utils::ChooseSwapPresentMode(presentModes);
		m_SupportDetails.Capabilities = capabilities;

		m_SupportDetails.MinImageCount = capabilities.minImageCount + 1;
		if (capabilities.maxImageCount > 0 && m_SupportDetails.MinImageCount > capabilities.maxImageCount) { // ?
			m_SupportDetails.MinImageCount = capabilities.maxImageCount;
		}
	}

	// Can specify what type of gpu is best
	bool PhysicalDevice::IsDeviceSuitable(VkPhysicalDevice device)
	{
		auto surface = VulkanContext::GetSurface();

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
			MUR_VK_ASSERT(vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport));

			if (presentSupport)
				m_QueueFamilyIndices.PresentFamily = i;

			if (m_QueueFamilyIndices.IsComplete())
				break;

			i++;
		}

		VkPhysicalDeviceFeatures supportedFeatures;
		vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

		return m_QueueFamilyIndices.IsComplete() &&  supportedFeatures.samplerAnisotropy;
		/*VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceProperties(m_NativePhysicalDevice, &deviceProperties);
		vkGetPhysicalDeviceFeatures(m_NativePhysicalDevice, &deviceFeatures);

		return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
			deviceFeatures.geometryShader;*/
	}}