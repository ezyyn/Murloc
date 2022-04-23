#include "murpch.hpp"

#include "VulkanContext.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Murloc {

	VulkanContext::VulkanContext(GLFWwindow* window)
		: m_NativeWindow(window)
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

	VulkanContext::~VulkanContext()
	{
		vkDestroySurfaceKHR(m_VulkanInstance->GetNative(), m_VulkanSurface, nullptr);
	}

	std::vector<const char*> VulkanContext::GetExtensions()
	{
		return m_Extensions;
	};

	void VulkanContext::CreateWindowSurface(const Ref<VulkanInstance>& instance)
	{
		m_VulkanInstance = instance;
		MUR_VK_ASSERT(glfwCreateWindowSurface(m_VulkanInstance->GetNative(), m_NativeWindow, nullptr, &m_VulkanSurface));
	}

	std::pair<uint32_t, uint32_t> VulkanContext::GetFrameBufferSize()
	{
		int width, height;
		glfwGetFramebufferSize(m_NativeWindow, &width, &height); // To be abstracted

		MUR_CORE_ASSERT(width >= 0 && height >= 0);

		return { (uint32_t)width, (uint32_t)height };
	}

}