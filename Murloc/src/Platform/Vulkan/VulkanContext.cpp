#include "murpch.hpp"

#include "VulkanContext.hpp"

#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>

namespace Murloc {

	VulkanContext::VulkanContext(GLFWwindow* window)
		: m_Window(window)
	{
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

		MUR_CORE_INFO("Vulkan extensions supported: {0}", extensionCount);

		uint32_t count = 0;
		const char** exts = glfwGetRequiredInstanceExtensions(&count);

		m_Extensions = std::vector<const char*>(exts, exts + count);

		for (uint32_t i = 0; i < m_Extensions.size(); ++i)
		{
			const char* name = m_Extensions[i];
			MUR_CORE_INFO("{0}", name);
		}

		std::vector<VkExtensionProperties> extensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
	}

	std::vector<const char*> VulkanContext::GetExtensions()
	{
		return m_Extensions;
	};

}