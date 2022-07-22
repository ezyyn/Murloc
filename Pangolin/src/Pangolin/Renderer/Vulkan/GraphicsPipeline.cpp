#include "pgpch.h"
#include "GraphicsPipeline.h"

#include "VulkanContext.h"

namespace PG {
    
	GraphicsPipeline::GraphicsPipeline()
	{
		VulkanContext::GetContextResourceFreeQueue().PushBack(PIPELINE, [this]()
			{
				CleanUp();
			});
	}
    
	GraphicsPipeline::~GraphicsPipeline()
	{
	}
    
	void GraphicsPipeline::Invalidate(VkRenderPass renderPass, const Ref<Shader>& shader)
	{
		if (m_NativePipeline) {
			CleanUp();
		}
        
		PG_CORE_WARN("Recreating graphics pipeline!");
        
		// TODO: Maybe for loop?
		// Vertex
		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = shader->GetShaderModules()[ShaderStage::VERTEX];
		vertShaderStageInfo.pName = "main";
		//Fragment 
		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = shader->GetShaderModules()[ShaderStage::FRAGMENT];
		fragShaderStageInfo.pName = "main";
        
		VkPipelineShaderStageCreateInfo shaderStagesCreateInfo[] = { vertShaderStageInfo, fragShaderStageInfo };
        
		// Vertex input (glVertexAttribArray vulkan version)
		auto& bufferLayout = shader->GetVertexBufferLayout();
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.pVertexAttributeDescriptions = bufferLayout.GetAttributesDescription().data(); // Optional
		vertexInputInfo.vertexAttributeDescriptionCount = bufferLayout.Size();
		VkVertexInputBindingDescription bindingDescription{};
		{
			bindingDescription.binding = 0;
			bindingDescription.stride = bufferLayout.GetStride();
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			vertexInputInfo.vertexBindingDescriptionCount = 1;
			vertexInputInfo.pVertexBindingDescriptions = &bindingDescription; // Optional
		}
        
		// Index buffer specification
		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;
        
		// Viewport state
		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = nullptr;
		viewportState.scissorCount = 1;
		viewportState.pScissors = nullptr;
        
		// Rasterizer
		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f; // Optional
		rasterizer.depthBiasClamp = 0.0f; // Optional
		rasterizer.depthBiasSlopeFactor = 0.0f; // Optional
        
		// Multisamlping
		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f; // Optional
		multisampling.pSampleMask = nullptr; // Optional
		multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
		multisampling.alphaToOneEnable = VK_FALSE; // Optional
		// Depth and stencil testing
		VkPipelineDepthStencilStateCreateInfo depthAndStencil{};
		// Color blending
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_TRUE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional
		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		//colorBlending.logicOpEnable = VK_FALSE;
		//colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
		//colorBlending.attachmentCount = 1;
		//colorBlending.blendConstants[0] = 0.0f; // Optional
		//colorBlending.blendConstants[1] = 0.0f; // Optional
		//colorBlending.blendConstants[2] = 0.0f; // Optional
		//colorBlending.blendConstants[3] = 0.0f; // Optional
        
		// Pipeline layout
		// Pipeline layout
		// Pipeline layout
		// Uniforms, samplers, etc.
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		auto& layouts = shader->GetDescriptorSetLayouts();
		pipelineLayoutInfo.setLayoutCount = (uint32_t)layouts.size(); // Optional
		pipelineLayoutInfo.pSetLayouts = layouts.data(); // Optional
		pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
		pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional
        
		PG_VK_ASSERT(vkCreatePipelineLayout(VulkanContext::GetLogicalDevice()->GetNative(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout));
        
		// Dynamic states
		VkDynamicState states[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
        
		VkPipelineDynamicStateCreateInfo dynamicStatesInfo{};
		dynamicStatesInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicStatesInfo.pDynamicStates = states;
		dynamicStatesInfo.flags = 0;
		dynamicStatesInfo.dynamicStateCount = 2;
        
		// Pipeline creation
		// Pipeline creation
		// Pipeline creation
		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStagesCreateInfo;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = nullptr; // Optional
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicStatesInfo;
		pipelineInfo.layout = m_PipelineLayout;
		pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional // boost for performance
		pipelineInfo.basePipelineIndex = -1; // Optional
        
		PG_VK_ASSERT(vkCreateGraphicsPipelines(VulkanContext::GetLogicalDevice()->GetNative(), 
			m_PipelineCache, 1, &pipelineInfo, nullptr, &m_NativePipeline));
	}
    
	void GraphicsPipeline::Bind(VkCommandBuffer cmdBuffer)
	{
		vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_NativePipeline);
	}
    
	void GraphicsPipeline::CleanUp()
	{
		if (!m_NativePipeline)
			return;
        
		vkDestroyPipeline(VulkanContext::GetLogicalDevice()->GetNative(), m_NativePipeline, nullptr);
		vkDestroyPipelineLayout(VulkanContext::GetLogicalDevice()->GetNative(), m_PipelineLayout, nullptr);
        
		m_NativePipeline = VK_NULL_HANDLE;
		m_PipelineLayout = VK_NULL_HANDLE;
	}
}