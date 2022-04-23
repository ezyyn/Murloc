#pragma once

#include "Murloc/Core/Window.hpp"

#include "Platform/Vulkan/VulkanInstance.hpp"

struct GLFWwindow;

namespace Murloc {

	class VulkanContext {
	public:

		VulkanContext(GLFWwindow* window);
		~VulkanContext();

		std::vector<const char*> GetExtensions();

		void CreateWindowSurface(const Ref<VulkanInstance>& instance);

		std::pair<uint32_t, uint32_t> GetFrameBufferSize();

		VkSurfaceKHR GetSurface() const { return m_VulkanSurface; };
	private:
		GLFWwindow* m_NativeWindow;
		std::vector<const char*> m_Extensions;

		Ref<VulkanInstance> m_VulkanInstance;
		VkSurfaceKHR m_VulkanSurface;
	};

}