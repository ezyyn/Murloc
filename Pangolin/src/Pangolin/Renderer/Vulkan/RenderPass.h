#pragma once

#include <vulkan/vulkan.h>

namespace PG 
{
	struct RenderPassInfo {

	};
	// TODO: REBUILD
	class RenderPass 
	{
	public:
		RenderPass(const RenderPassInfo& info = {});
		RenderPass(const VkRenderPassCreateInfo& info);
		RenderPass(VkRenderPass renderPass);
		~RenderPass();

		VkRenderPass GetNative() const { return m_RenderPass; }

		const RenderPassInfo& GetInfo() const { return m_RenderPassInfo; }
	private:
		RenderPassInfo m_RenderPassInfo;
		VkRenderPass m_RenderPass;
	};
}