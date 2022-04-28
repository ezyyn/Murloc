#include "murpch.hpp"

#include "VulkanDevice.hpp"
#include "Vulkan.hpp"


namespace Murloc {

	VulkanDevice::VulkanDevice()
	{
		m_PhysicalDevice = CreateRef<VulkanPhysicalDevice>();

		const auto& indices = m_PhysicalDevice->GetQueueFamilyIndices();

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

		std::vector<const char*> validationLayers {
			"VK_LAYER_KHRONOS_validation"
		};

		if (Vulkan::ValidationLayersEnabled()) {
			// Extension layers

			createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
			createInfo.ppEnabledExtensionNames = extensions.data();

			// Validation layers
			validationLayers.push_back("VK_LAYER_KHRONOS_validation");

			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}

		MUR_VK_ASSERT(vkCreateDevice(m_PhysicalDevice->GetNative(), &createInfo, nullptr, &m_Device));

		vkGetDeviceQueue(m_Device, indices.GraphicsFamily.value(), 0, &m_GraphicsQueue);
		vkGetDeviceQueue(m_Device, indices.PresentFamily.value(), 0, &m_PresentQueue);

		// Command pool
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = m_PhysicalDevice->GetQueueFamilyIndices().GraphicsFamily.value();

		MUR_VK_ASSERT(vkCreateCommandPool(m_Device, &poolInfo, nullptr, &m_CommandPool));
	}

	VulkanDevice::~VulkanDevice()
	{
		vkDestroyCommandPool(m_Device, m_CommandPool, nullptr);
		vkDestroyDevice(m_Device, nullptr);
	}

}