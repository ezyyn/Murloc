#pragma once

#include <vulkan/vulkan.h>

#include "Murloc/Renderer/Vulkan/Device.h"
#include "Murloc/Renderer/CommandQueue.h"

struct GLFWwindow;

namespace Murloc {

	class VulkanContext {
	public:
		static void Init();
		static void Shutdown();

		[[nodiscard]] static VkInstance GetVulkanInstance();
		[[nodiscard]] static VkSurfaceKHR GetSurface();
		[[nodiscard]] static LogicalDevice* GetLogicalDevice();
		[[nodiscard]] static CommandQueue& GetContextResourceFreeQueue();

		static bool ValidationLayersEnabled();
	private:
		static void CreateInstance();
		static void CreateDebugger();
		static void CreateSurface();

		friend class LogicalDevice;
	};

}