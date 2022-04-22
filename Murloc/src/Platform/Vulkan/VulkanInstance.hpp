#pragma once

#include "Platform/Vulkan/Vulkan.hpp"

namespace Murloc {

	struct VulkanDebuggerParams;

	class VulkanInstance {
	public:
		VulkanInstance(std::vector<const char*>& extensions);
		~VulkanInstance();

		VkInstance GetNative() const { return m_VulkanInstance; }

		std::vector<const char*> GetValidationLayers() { return m_ValidationLayers; }
	private:
		VkInstance m_VulkanInstance{ VK_NULL_HANDLE };
		std::vector<const char*> m_ValidationLayers{};
	};
}