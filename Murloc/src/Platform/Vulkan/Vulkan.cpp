#include "murpch.hpp"

#include "Vulkan.hpp"

#include "Platform/Vulkan/VulkanContext.hpp"
#include "Platform/Vulkan/VulkanUtils.hpp"
#include "Platform/Vulkan/VulkanDebug.hpp"
#include "Platform/Vulkan/VulkanPhysicalDevice.hpp"
#include "Platform/Vulkan/VulkanDevice.hpp"
#include "Platform/Vulkan/VulkanContext.hpp"
#include "Platform/Vulkan/VulkanSwapchain.hpp"

namespace Murloc {

	struct VulkanObjects {

		Ref<VulkanDebug> Debugger;
		Ref<VulkanInstance> Instance;
		Ref<VulkanDevice> Device;
		Ref<VulkanContext> Context;
		Ref<VulkanSwapchain> Swapchain;

		~VulkanObjects() {
			Device.reset();
			Debugger.reset();
			Context.reset();
			Instance.reset();
		}
	};

	void Vulkan::Init(GLFWwindow* window)
	{
		s_VulkanObjects = new VulkanObjects;
		
		s_VulkanObjects->Context = CreateRef<VulkanContext>(window);

		s_VulkanObjects->Instance = CreateRef<VulkanInstance>(s_VulkanObjects->Context->GetExtensions());

		s_VulkanObjects->Context->CreateWindowSurface(s_VulkanObjects->Instance);

		if (s_EnableValidationLayer) { 
			// VkInstance must be created before creating VulkanDebugger
			s_VulkanObjects->Debugger = CreateRef<VulkanDebug>(s_VulkanObjects->Instance);
		}

		s_VulkanObjects->Device = CreateRef<VulkanDevice>(s_VulkanObjects->Instance, s_VulkanObjects->Context);

		s_VulkanObjects->Swapchain = CreateRef<VulkanSwapchain>(s_VulkanObjects->Device, s_VulkanObjects->Context);
	}

	void Vulkan::Shutdown()
	{
		delete s_VulkanObjects;
	}
}