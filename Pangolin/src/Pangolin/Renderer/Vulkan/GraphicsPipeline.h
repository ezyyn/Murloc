#pragma once

#include <vulkan/vulkan.h>

#include "Shader.h"
#include "Pangolin/Renderer/BufferLayout.h"

namespace PG {
	
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

		VkPipelineLayout GetPipelineLayout() const { return m_PipelineLayout; };

		static inline VkPipeline m_NativePipeline{ VK_NULL_HANDLE };
	private:
		void CleanUp();

		VkPipelineLayout m_PipelineLayout{ VK_NULL_HANDLE };

		VkPipelineCache m_PipelineCache{ VK_NULL_HANDLE };
	};

}