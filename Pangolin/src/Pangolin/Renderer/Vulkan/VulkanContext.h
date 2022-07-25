#pragma once

#include <vulkan/vulkan.h>

#include "Pangolin/Renderer/Vulkan/Device.h"
#include "Pangolin/Renderer/CommandQueue.h"

struct GLFWwindow;

namespace PG {

	class VulkanContext {
	public:
		static void Init();
		static void Shutdown();

		static VkInstance GetVulkanInstance();
		static VkSurfaceKHR GetSurface();
		static LogicalDevice* GetLogicalDevice();
		static CommandQueue& GetContextResourceFreeQueue();

		static bool ValidationLayersEnabled();
	private:
		static void CreateInstance();
		static void CreateDebugger();
		static void CreateSurface();

		friend class LogicalDevice;
	};

}