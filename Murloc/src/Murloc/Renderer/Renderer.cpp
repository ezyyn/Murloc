#include "murpch.hpp"

#include "Murloc/Renderer/Renderer.hpp"

#include "Murloc/Renderer/Vulkan/Shader.h"
#include "Murloc/Renderer/Vulkan/Swapchain.h"
#include "Murloc/Renderer/Vulkan/GraphicsPipeline.h"
#include "Murloc/Renderer/Vulkan/Texture.h"
#include "Murloc/Renderer/Vulkan/VulkanContext.h"
#include "Murloc/Renderer/Vulkan/VulkanBuffer.h"
#include "Murloc/Renderer/RenderCommand.h"
#include "Murloc/Renderer/ShaderLibrary.h"

#include "Murloc/ImGui/ImGuiLayer.hpp"

#include "Murloc/Core/Application.hpp"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>

#include <imgui.h>

// BatchRenderer

namespace Murloc {
    
	struct QuadVertex {
		glm::vec3 Position;
		glm::vec4 Color;
		glm::vec2 TextureCoord;
		int TexIndex;
	};
    
	// Stats
	struct Statistics
	{
		uint32_t DrawCalls = 0;
		uint32_t QuadCount = 0;

		uint32_t GetTotalVertexCount() const { return QuadCount * 4; }
		uint32_t GetTotalIndexCount() const { return QuadCount * 6; }
	};

	struct RenderData
	{
		static const uint32_t MaxQuads = 50;
		static const uint32_t MaxVertices = MaxQuads * 4;
		static const uint32_t MaxIndices = MaxQuads * 6;
		static const uint32_t MaxTextureSlots = 32; // TODO: RenderCaps

		// Quad
		Ref<VertexBuffer> QuadVertexBuffer;
		Ref<IndexBuffer> QuadIndexBuffer;
		Ref<Shader> QuadShader;

		uint32_t QuadIndexCount = 0;
		QuadVertex* QuadVertexBufferBase = nullptr;
		QuadVertex* QuadVertexBufferPtr = nullptr;

		Ref<GraphicsPipeline> FlatColorPipeline;
        
		Ref<Texture> TestTexture;

		Statistics Stats;
	};
    
	struct UniformBufferData {
		glm::mat4 Model;
		glm::mat4 View;
		glm::mat4 Projection;
	};
    
	static RenderData s_Data;
    
    void Renderer::Init()
    {
       /* const std::vector<QuadVertex> vertices = {
			{ {-0.5f,-0.5f, 0.0f}, { 1.0f, 0.0f, 0.0f, 1.0f} , { 1.0f, 0.0f } },
			{ { 0.5f, -0.5f, 0.0f}, { 1.0f, 1.0f, 1.0f, 1.0f}, { 0.0f, 0.0f } },
			{ { 0.5f, 0.5f, 0.0f}, { 0.0f, 1.0f, 0.0f, 1.0f} , { 0.0f, 1.0f } },
			{ { -0.5f, 0.5f, 0.0f}, { 0.0f, 1.0f, 0.0f, 1.0f}, { 1.0f, 1.0f } },
        };

        const std::vector<uint32_t> indices = {
            0, 1, 2, 2, 3, 0
        };*/

        s_Data.QuadVertexBuffer = CreateRef<VertexBuffer>(s_Data.MaxVertices * sizeof(QuadVertex));

		{
			s_Data.QuadVertexBufferBase = new QuadVertex[s_Data.MaxVertices];
			uint32_t* quadIndices = new uint32_t[s_Data.MaxIndices];
			uint32_t offset = 0;
			for (uint32_t i = 0; i < s_Data.MaxIndices; i += 6)
			{
				quadIndices[i + 0] = offset + 0;
				quadIndices[i + 1] = offset + 1;
				quadIndices[i + 2] = offset + 2;

				quadIndices[i + 3] = offset + 2;
				quadIndices[i + 4] = offset + 3;
				quadIndices[i + 5] = offset + 0;

				offset += 4;
			}
			s_Data.QuadIndexBuffer = CreateRef<IndexBuffer>(quadIndices, s_Data.MaxIndices);
			delete[] quadIndices;
		}
		
        //s_Data.QuadVertexBuffer->SetData((void*)vertices.data());

        s_Data.TestTexture = CreateRef<Texture>("assets/textures/texture.jpg");

        //ShaderLibrary::Init("assets/shaders");

        {
            // Describing binding in fragment shader(sampler)
            VkDescriptorSetLayoutBinding samplerLayoutBinding{};
            samplerLayoutBinding.binding = 1;
            samplerLayoutBinding.descriptorCount = 1;
            samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            samplerLayoutBinding.pImmutableSamplers = nullptr;
            samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        }

        s_Data.QuadShader = CreateRef<Shader>("assets/shaders/Texture.glsl");

        s_Data.FlatColorPipeline = CreateRef<GraphicsPipeline>();
        s_Data.FlatColorPipeline->Invalidate(Swapchain::s_RenderPass,
            s_Data.QuadShader);
    }
    
	void Renderer::Shutdown()
	{
		// Wait for the GPU to finish its work
		RenderCommand::WaitIdle();
        
		delete[] s_Data.QuadVertexBufferBase;

		s_Data.TestTexture.reset();
	}
    
	void Renderer::OnResize(uint32_t width, uint32_t height)
	{
		//const auto swapChain = (Swapchain*)Application::Get()->GetWindow()->GetSwapchain();
		//swapChain->Invalidate();

		//s_Data.FlatColorPipeline->Invalidate(Swapchain::s_RenderPass, s_Data.TextureShader, s_Data.DescriptorSetLayout);
	}

	UniformBufferData ubo{};

	void Renderer::BeginScene()
	{
		// Camera setup
        //...
		StartBatch();
	}
    
	void Renderer::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
	{
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
			* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

		static constexpr glm::vec4 quadVertexPositions[4] = {
			{ -0.5f, -0.5f, 0.0f, 1.0f },
			{ 0.5f, -0.5f, 0.0f , 1.0f  },
			{ 0.5f,  0.5f, 0.0f , 1.0f  },
			{ -0.5f,  0.5f, 0.0f, 1.0f }
		};

		constexpr size_t quadVertexCount = 4;
		const float textureIndex = 0.0f; // White Texture
		constexpr glm::vec2 textureCoords[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };
		const float tilingFactor = 1.0f;

		/*if (s_Data.QuadIndexCount >= Renderer2DData::MaxIndices)
			NextBatch();*/

		for (size_t i = 0; i < quadVertexCount; i++)
		{
			s_Data.QuadVertexBufferPtr->Position = transform * quadVertexPositions[i];
			s_Data.QuadVertexBufferPtr->Color = color;
			s_Data.QuadVertexBufferPtr->TextureCoord = textureCoords[i];
			s_Data.QuadVertexBufferPtr->TexIndex = 0;
			//s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
			//s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
			//s_Data.QuadVertexBufferPtr->EntityID = entityID;
			s_Data.QuadVertexBufferPtr++;
		}

		s_Data.QuadIndexCount += 6;

		s_Data.Stats.QuadCount++;

	}

	void Renderer::EndScene()
	{
		Flush();
	}

	void Renderer::StartBatch()
	{
		s_Data.QuadIndexCount = 0;
		s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;
	}

	void Renderer::Flush()
	{
		const auto swapChain = (Swapchain*)Application::Get()->GetWindow()->GetSwapchain();

		// Update/Upload uniform buffers
		{
			static auto startTime = std::chrono::high_resolution_clock::now();

			auto currentTime = std::chrono::high_resolution_clock::now();
			float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

			ubo.Model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			ubo.View = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			ubo.Projection = glm::perspective(glm::radians(45.0f),
				swapChain->GetExtent().width / (float)swapChain->GetExtent().height, 0.1f, 10.0f);

			ubo.Projection[1][1] *= -1;

			// u_MVP
			s_Data.QuadShader->SetUniformData(0, &ubo);
		}
		uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.QuadVertexBufferPtr - (uint8_t*)s_Data.QuadVertexBufferBase);
		s_Data.QuadVertexBuffer->SetData(s_Data.QuadVertexBufferBase, dataSize);

		//if (s_Data.QuadIndexCount)
		{
			auto currentRenderCmdBuffer = Swapchain::s_CurrentRenderCommandbuffer;
			auto currentFramebuffer = Swapchain::s_CurrentFramebuffer;

			auto indexCount = s_Data.QuadIndexCount;
			auto& pipeline = s_Data.FlatColorPipeline;
			auto& shader = s_Data.QuadShader;

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

				renderPassInfo.clearValueCount = 1;
				VkClearValue clearValue{};
				clearValue.color = { 0.2f,0.2f, 0.2f, 1.0f };
				renderPassInfo.pClearValues = &clearValue;
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
					scissor.offset = { 0, 0 };
					scissor.extent = extent;
					vkCmdSetViewport(currentRenderCmdBuffer, 0, 1, &viewport);
					vkCmdSetScissor(currentRenderCmdBuffer, 0, 1, &scissor);
				}

				s_Data.FlatColorPipeline->Bind(currentRenderCmdBuffer);

				VkBuffer vertexBuffers[]{ s_Data.QuadVertexBuffer->GetNative() };
				VkDeviceSize offsets[] = { 0 };
				vkCmdBindVertexBuffers(currentRenderCmdBuffer, 0, 1, vertexBuffers, offsets);
				vkCmdBindIndexBuffer(currentRenderCmdBuffer, s_Data.QuadIndexBuffer->GetNative(), 0, VK_INDEX_TYPE_UINT32);

				auto sets = shader->GetCurrentDescriptorSet();

				vkCmdBindDescriptorSets(currentRenderCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
					pipeline->GetPipelineLayout(), 0, 1, &sets, 0, nullptr);

				vkCmdDrawIndexed(currentRenderCmdBuffer, indexCount, 1, 0, 0, 0);

				// RenderPass End
				// RenderPass End
				// RenderPass End
				vkCmdEndRenderPass(currentRenderCmdBuffer);

				MUR_VK_ASSERT(vkEndCommandBuffer(currentRenderCmdBuffer));
			}
		}
	}

}