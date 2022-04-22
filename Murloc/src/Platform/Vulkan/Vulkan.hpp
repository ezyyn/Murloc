#pragma once

#include <vulkan/vulkan.hpp>

namespace Murloc {

	struct VulkanObjects;
	class VulkanContext;

	class Vulkan {
	public:
		static void Init(VulkanContext* context);
		static void Shutdown();

		static bool ValidationLayerEnabled() { return s_EnableValidationLayer; }

	private:
		Vulkan() = delete;
		Vulkan(const Vulkan&) = delete;
		Vulkan(Vulkan&&) = delete;

		static inline bool s_EnableValidationLayer{ true };
		static inline VulkanObjects* s_VulkanObjects{ nullptr };
	};
}