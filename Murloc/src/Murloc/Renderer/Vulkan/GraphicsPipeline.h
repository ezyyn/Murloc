#pragma once

#include <vulkan/vulkan.h>

#include "Shader.h"
#include "Murloc/Renderer/BufferLayout.hpp"

namespace Murloc {
	
	struct GraphicsPipelineSpecification 
	{
	};

	// Manager class
	class GraphicsPipeline {
	public:
		GraphicsPipeline();
		~GraphicsPipeline();

		void Invalidate(VkRenderPass renderPass, const Ref<Shader>& shader);

		void Bind(VkCommandBuffer buffer);
		void CleanUp();

		VkPipelineLayout GetPipelineLayout() const { return m_PipelineLayout; };
	private:
		VkPipelineLayout m_PipelineLayout{ VK_NULL_HANDLE };

		VkPipelineCache m_PipelineCache{ VK_NULL_HANDLE };
		VkPipeline m_NativePipeline{ VK_NULL_HANDLE };
	};

}