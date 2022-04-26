#pragma once

#include "Murloc/Core/Common.hpp"
#include "Platform/Vulkan/VulkanDevice.hpp"

#include <vulkan/vulkan.h>

namespace Murloc {

	class Vulkan {
	public:
		static void Init(bool validationLayers = true);
		static void Shutdown();

		static VkInstance GetVulkanInstance();
		static VkSurfaceKHR GetSurface();
		static const Ref<VulkanDevice>& GetDevice();

		static bool ValidationLayersEnabled();

		static std::pair<uint32_t, uint32_t> GetFramebufferSize();
	private:
		static void CreateInstance();
		static void CreateDebugger();
	};
}