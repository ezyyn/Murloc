#include "murpch.hpp"

#include "BatchRenderer.h"

#include "Murloc/Core/Application.hpp"

#include "Vulkan/VertexBuffer.h"
#include "Vulkan/IndexBuffer.h"
#include "Vulkan/VulkanBuffer.h"
#include "Vulkan/RenderPass.h"
#include "Vulkan/Swapchain.h"
#include "Vulkan/Texture.h"
#include "Vulkan/VulkanContext.h"

#include "CommandQueue.h"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Murloc {
    
    struct QuadVertex {
        glm::vec3 Position;
        glm::vec4 Color;
        glm::vec2 TextureCoord;
    };

	// Stats
	struct Statistics
	{
		uint32_t DrawCalls = 0;
		uint32_t QuadCount = 0;

		uint32_t GetTotalVertexCount() const { return QuadCount * 4; }
		uint32_t GetTotalIndexCount() const { return QuadCount * 6; }
	};

    struct RenderData {
        Ref<VertexBuffer> QuadVertexBuffer;
        Ref<IndexBuffer> QuadIndexBuffer;


		Ref<Texture> TestTexture;

		std::vector<Ref<VulkanBuffer>> UniformBuffers;
    };

	struct MVP_UBO {
		glm::mat4 Model;
		glm::mat4 View;
		glm::mat4 Projection;
	} mvpUBO;

    static CommandQueue s_RendererResourceFreeQueue;

	BatchRenderer::BatchRenderer(const RendererInfo& info /*= {}*/)
        : m_RendererInfo(info)
	{
		m_Data = new RenderData;
        
        Init();
	}

	BatchRenderer::~BatchRenderer()
    {
        Shutdown();
    }

    void BatchRenderer::Init()
    {
        const std::vector<QuadVertex> vertices = {
           { {-0.5f,-0.5f, 0.0f}, { 1.0f, 0.0f, 0.0f, 1.0f} , { 1.0f, 0.0f } },
           { { 0.5f, -0.5f, 0.0f}, { 1.0f, 1.0f, 1.0f, 1.0f}, { 0.0f, 0.0f } },
           { { 0.5f, 0.5f, 0.0f}, { 0.0f, 1.0f, 0.0f, 1.0f} , { 0.0f, 1.0f } },
           { { -0.5f, 0.5f, 0.0f}, { 0.0f, 1.0f, 0.0f, 1.0f}, { 1.0f, 1.0f } },
        };

        const std::vector<uint32_t> indices = {
            0, 1, 2, 2, 3, 0
        };

		m_Data->QuadIndexBuffer = CreateRef<IndexBuffer>(indices.data(), 6);
		m_Data->QuadVertexBuffer = CreateRef<VertexBuffer>(vertices.size() * sizeof(QuadVertex));
		//m_Data->QuadVertexBuffer->SetData((void*)vertices.data());
		m_Data->TestTexture = CreateRef<Texture>("assets/textures/texture.jpg");

       /* m_RendererInfo.Shader = CreateRef<Shader>("assets/shaders/Texture.glsl",
			BufferLayout
			{
				{ShaderDataType::Float3, "a_Position"},
				{ShaderDataType::Float4, "a_Color"},
				{ShaderDataType::Float2, "a_TexCoords"}
			});*/
    }

	void BatchRenderer::BeginScene()
	{
        // Start batch ...
	}

	void BatchRenderer::EndScene()
	{
        // Draw scene

		const auto swapChain = (Swapchain*)Application::Get()->GetWindow()->GetSwapchain();

		// Update/Upload uniform buffers
		{
			static auto startTime = std::chrono::high_resolution_clock::now();

			auto currentTime = std::chrono::high_resolution_clock::now();
			float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

			mvpUBO.Model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			mvpUBO.View = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			mvpUBO.Projection = glm::perspective(glm::radians(45.0f),
				swapChain->GetExtent().width / (float)swapChain->GetExtent().height, 0.1f, 10.0f);

			mvpUBO.Projection[1][1] *= -1;

			m_Data->UniformBuffers[swapChain->s_CurrentFrame]->SetData(&mvpUBO);
		}

		//VkDescriptorSet currentDescriptorSets[2] = { s_Data.DescriptorSets[swapChain->m_CurrentFrame], s_Data.TestTexture->GetDescriptorSets()[swapChain->m_CurrentFrame]};

		/*RenderCommand::SetClearValue({ {0.2f,0.2f,0.2f,1.0f} });
		RenderCommand::DrawIndexed(s_Data.QuadVertexBuffer, s_Data.QuadIndexBuffer, s_Data.FlatColorPipeline,
			s_Data.DescriptorSets[swapChain->m_CurrentFrame], 6);*/

		auto currentRenderCmdBuffer = Swapchain::s_CurrentRenderCommandbuffer;
		auto currentFramebuffer = Swapchain::s_CurrentFramebuffer;

		auto& appSpec = Application::Get()->GetSpecification();
		VkExtent2D extent = { appSpec.Width, appSpec.Height };

		// CommandBuffers
		// CommandBuffers
		// CommandBuffers
		{
			vkResetCommandBuffer(currentRenderCmdBuffer, 0);

			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // Optional
			beginInfo.pInheritanceInfo = nullptr; // Optional
			MUR_VK_ASSERT(vkBeginCommandBuffer(currentRenderCmdBuffer, &beginInfo));
		}

		// RenderPass Begin
		// RenderPass Begin
		// RenderPass Begin
		{
			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = Swapchain::s_RenderPass;
			renderPassInfo.framebuffer = currentFramebuffer;
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = extent;

			VkClearValue clearColor = { 0.2f,0.2f, 0.2f, 1.0f };

			renderPassInfo.clearValueCount = 1;
			renderPassInfo.pClearValues = &clearColor; // Clear color
			vkCmdBeginRenderPass(currentRenderCmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		}

		{
			{
				// Dynamic viewport
				VkViewport viewport{};
				viewport.x = 0.0f;
				viewport.y = 0.0f;
				viewport.width = (float)extent.width;
				viewport.height = (float)extent.height;
				viewport.minDepth = 0.0f;
				viewport.maxDepth = 1.0f;
				VkRect2D scissor{};
				scissor.offset = { 0, 0 }; // TODO: Store render into texture and then display is into ImGui
				scissor.extent = extent;
				vkCmdSetViewport(currentRenderCmdBuffer, 0, 1, &viewport);
				vkCmdSetScissor(currentRenderCmdBuffer, 0, 1, &scissor);
			}

			m_RendererInfo.Pipeline->Bind(currentRenderCmdBuffer);

			VkBuffer vertexBuffers[]{ m_Data->QuadVertexBuffer->GetNative() };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(currentRenderCmdBuffer, 0, 1, vertexBuffers, offsets);
			vkCmdBindIndexBuffer(currentRenderCmdBuffer, m_Data->QuadIndexBuffer->GetNative(), 0, VK_INDEX_TYPE_UINT32);
			//vkCmdBindDescriptorSets(currentRenderCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			//m_RendererInfo.Pipeline->GetPipelineLayout(), 0, 1, &currentDescriptorSet, 0, nullptr);

			vkCmdDrawIndexed(currentRenderCmdBuffer, 6, 1, 0, 0, 0);

			// RenderPass End
			// RenderPass End
			// RenderPass End
			vkCmdEndRenderPass(currentRenderCmdBuffer);

			MUR_VK_ASSERT(vkEndCommandBuffer(currentRenderCmdBuffer));
		}
	}

	void BatchRenderer::Shutdown()
	{
		vkDeviceWaitIdle(VulkanContext::GetLogicalDevice()->GetNative());

        s_RendererResourceFreeQueue.Execute();

        delete m_Data;
        m_Data = nullptr;
	}

}
