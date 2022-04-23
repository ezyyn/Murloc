#pragma once

#include "Murloc/Core/Common.hpp"

#include <vulkan/vulkan.hpp>

struct GLFWwindow;

namespace Murloc {

	class Vulkan {
	public:
		static void Init(GLFWwindow* window);
		static void Shutdown();

		static bool ValidationLayerEnabled() { return s_EnableValidationLayer; }
	private:
		Vulkan() = delete;
		Vulkan(const Vulkan&) = delete;
		Vulkan(Vulkan&&) = delete;

		static inline bool s_EnableValidationLayer{ true };
		static inline struct VulkanObjects* s_VulkanObjects{ nullptr };
	};
}