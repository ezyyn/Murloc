#pragma once

#include <vulkan/vulkan.h>

namespace PG {

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
		VkPhysicalDeviceProperties Properties;
	};

	class PhysicalDevice {
	public:
		PhysicalDevice();
		~PhysicalDevice();

		void PopulateSwapchainSupportDetails();

		bool IsDeviceSuitable(VkPhysicalDevice device);

		const SwapchainSupportDetails& GetSupportDetails() const { return m_SupportDetails; }

		const QueueFamilyIndices& GetQueueFamilyIndices() const { return m_QueueFamilyIndices; }

		VkPhysicalDevice GetNative() const { return m_NativePhysicalDevice; }
	private:
		QueueFamilyIndices m_QueueFamilyIndices;

		SwapchainSupportDetails m_SupportDetails;

		VkPhysicalDevice m_NativePhysicalDevice{ VK_NULL_HANDLE };
	};

	class LogicalDevice {
	public:
		LogicalDevice();
		~LogicalDevice();

		const PhysicalDevice& GetPhysicalDevice() const { return m_PhysicalDevice; }
		VkDevice GetNative() const { return m_NativeDevice; }

		VkCommandPool GetCommandPool() const { return m_CommandPool; }

		VkQueue GetPresentQueue() const { return m_PresentQueue; }
		VkQueue GetGraphicsQueue() const { return m_GraphicsQueue; }
		
	private:
		// One command pool (one thread) for now
		VkCommandPool m_CommandPool{ VK_NULL_HANDLE };

		VkQueue m_GraphicsQueue{ VK_NULL_HANDLE };
		VkQueue m_PresentQueue{ VK_NULL_HANDLE };

		VkDevice m_NativeDevice{ VK_NULL_HANDLE };

		PhysicalDevice m_PhysicalDevice;
	};

}