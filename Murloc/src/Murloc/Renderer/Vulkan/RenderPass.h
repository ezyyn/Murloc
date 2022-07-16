#pragma once

#include <vulkan/vulkan.h>

namespace Murloc 
{
	struct RenderPassSpecification 
	{

	};

	class RenderPass 
	{
	public:
		RenderPass(const RenderPassSpecification& specs = {});
		~RenderPass();
	private:
		RenderPassSpecification m_Specification;
		VkRenderPass m_RenderPass;
	};
}