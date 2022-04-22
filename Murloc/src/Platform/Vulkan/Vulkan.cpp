#include "murpch.hpp"

#include "Vulkan.hpp"

#include "Platform/Vulkan/VulkanContext.hpp"
#include "Platform/Vulkan/VulkanUtils.hpp"
#include "Platform/Vulkan/VulkanDebug.hpp"
#include "Platform/Vulkan/VulkanInstance.hpp"

namespace Murloc {

	struct VulkanObjects {

		Ref<VulkanDebug> Debugger;
		Ref<VulkanInstance> Instance;

		~VulkanObjects() {
			Debugger.reset();
			Instance.reset();
		}
	};

	void Vulkan::Init(VulkanContext* context)
	{
		s_VulkanObjects = new VulkanObjects;
		
		s_VulkanObjects->Instance = CreateRef<VulkanInstance>(context->GetExtensions());

		if (s_EnableValidationLayer) { 
			// VkInstance must be created before creating VulkanDebugger
			s_VulkanObjects->Debugger = CreateRef<VulkanDebug>(s_VulkanObjects->Instance);
		}
	}

	void Vulkan::Shutdown()
	{
		delete s_VulkanObjects;
	}

}