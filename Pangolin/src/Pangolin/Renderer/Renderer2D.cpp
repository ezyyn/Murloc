#include "pgpch.h"

#include "Pangolin/Renderer/Renderer2D.h"

#include "Pangolin/Renderer/Vulkan/Shader.h"
#include "Pangolin/Renderer/Vulkan/Swapchain.h"
#include "Pangolin/Renderer/Vulkan/GraphicsPipeline.h"
#include "Pangolin/Renderer/Vulkan/Texture.h"
#include "Pangolin/Renderer/Vulkan/VulkanContext.h"
#include "Pangolin/Renderer/Vulkan/VulkanBuffer.h"
#include "Pangolin/Renderer/Vulkan/VulkanUtils.h"
#include "Pangolin/Renderer/Vulkan/RenderPass.h"
#include "Pangolin/Renderer/Vulkan/Framebuffer.h"
#include "Pangolin/Renderer/Vulkan/VertexBuffer.h"
#include "Pangolin/Renderer/Vulkan/IndexBuffer.h"
#include "Pangolin/Renderer/Vulkan/Image2D.h"

#include "Pangolin/Entities/Components.h"

#define PG_VULKAN_FUNCTIONS
#include "Pangolin/Renderer/RenderManager.h"

#include "Pangolin/Core/Application.h"
#include "Pangolin/Core/Timer.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>

#include "Pangolin/Renderer/Camera.h"

// BatchRenderer

namespace PG {
    
	struct QuadVertex {
		glm::vec3 Position;
		glm::vec4 Color;
		glm::vec2 TextureCoord;
		float TexIndex;
	};

	struct RenderData
	{
		static constexpr uint32_t MaxQuads = 50;
		static constexpr uint32_t MaxVertices = MaxQuads * 4;
		static constexpr uint32_t MaxIndices = MaxQuads * 6;
		static constexpr uint32_t MaxTextureSlots = 32; // TODO: Render Capabilities

		// Quad
		Ref<VertexBuffer> QuadVertexBuffer;
		Ref<IndexBuffer> QuadIndexBuffer;
		Ref<Shader> QuadShader;

		uint32_t QuadIndexCount = 0;
		QuadVertex* QuadVertexBufferBase = nullptr;
		QuadVertex* QuadVertexBufferPtr = nullptr;

		Ref<GraphicsPipeline> FlatColorPipeline;
        
		Ref<Texture2D> WhiteTexture;
		Ref<Texture2D> TestTexture2;
		Ref<Texture2D> TestTexture3;

		Renderer2D::Statistics Stats;

		// Renderer specification
		VkSampler TextureSampler{ VK_NULL_HANDLE };
		std::vector<VkDescriptorSet> ImGuiTextureDescriptorSets;
		Ref<RenderPass> Renderpass;
		std::vector<Ref<Framebuffer>> Framebuffers;
		std::vector<VkCommandBuffer> RenderToTextureCommandBuffers;
		std::vector<Ref<Image2D>> RenderImages;
		Ref<Image2D> DepthImage;
	};
    
	void Renderer2D::Init_Internal()
	{
		auto framesInFlight = RenderManager::GetSettings().FramesInFlight;

		CreateRenderPass();
		CreateTextureSampler();

		auto& windowSize = Application::Get()->GetSpecification();

		// Creating image for every frame in flight
		m_RenderData->RenderImages.resize(framesInFlight);
		for (size_t i = 0; i < m_RenderData->RenderImages.size(); ++i)
		{
			Image2DInfo info{};
			info.Format = VK_FORMAT_B8G8R8A8_UNORM;
			info.Aspect = VK_IMAGE_ASPECT_COLOR_BIT;
			info.Width = windowSize.Width;
			info.Height = windowSize.Height;
			info.Storage = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
			info.Usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
			m_RenderData->RenderImages[i] = CreateRef<Image2D>(info);
		}

		// Depth buffer
		Image2DInfo info{};
		info.Format = VK_FORMAT_D32_SFLOAT_S8_UINT;
		info.Aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
		info.Width = windowSize.Width;
		info.Height = windowSize.Height;
		info.Storage = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		info.Usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		m_RenderData->DepthImage = CreateRef<Image2D>(info);

		// Framebuffers
		// Framebuffers
		// Framebuffers
		m_RenderData->Framebuffers.resize(framesInFlight);
		for (uint32_t i = 0; i < framesInFlight; ++i)
		{
			FramebufferInfo info;
			info.ColorAttachment = m_RenderData->RenderImages[i]->GetImageView();
			info.DepthAttachment = m_RenderData->DepthImage->GetImageView();
			info.Width = windowSize.Width;
			info.Height = windowSize.Height;
			info.Renderpass = m_RenderData->Renderpass;

			m_RenderData->Framebuffers[i] = CreateRef<Framebuffer>(info);
		}
		// TODO: FIGURE OUT AM I SUPPOSE TO DO WITH THIS
		// Registering textures
		{
			auto device = VulkanContext::GetLogicalDevice();

			m_RenderData->ImGuiTextureDescriptorSets.resize(framesInFlight);

			for (size_t i = 0; i < m_RenderData->ImGuiTextureDescriptorSets.size(); ++i)
			{
				m_RenderData->ImGuiTextureDescriptorSets[i] = RenderManager::AllocateImGuiTextureDescriptorSet(m_RenderData->TextureSampler,
					m_RenderData->RenderImages[i]->GetImageView(),
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			}
		}
	}

	void Renderer2D::CreateTextureSampler()
	{
		auto device = VulkanContext::GetLogicalDevice()->GetNative();

		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

		samplerInfo.anisotropyEnable = VK_FALSE;
		samplerInfo.maxAnisotropy = 1.0f;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		PG_VK_ASSERT(vkCreateSampler(device, &samplerInfo, nullptr, &m_RenderData->TextureSampler));

		VulkanContext::GetContextResourceFreeQueue().PushBack(SAMPLER, [device, textureSampler = &m_RenderData->TextureSampler]()
		{
			vkDestroySampler(device, *textureSampler, nullptr);
		});

	}
	#pragma region CreateRenderPass
	void Renderer2D::CreateRenderPass()
	{
		VkRenderPass renderPass{};

		// Attachments
		std::vector<VkAttachmentDescription> attachments;
		std::vector<VkSubpassDependency> dependencies;

		VkSubpassDescription subpassDescription{};
		{
			// Attachment description
			auto& colorAttachment = attachments.emplace_back();
			colorAttachment.format = VK_FORMAT_B8G8R8A8_UNORM;
			colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			// Subpasses and attachment references
			VkAttachmentReference colorAttachmentRef{};
			colorAttachmentRef.attachment = 0;
			colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpassDescription.colorAttachmentCount = 1;
			subpassDescription.pColorAttachments = &colorAttachmentRef;
		}
		{
			// Depth buffer
			auto& depthAttachment = attachments.emplace_back();
			depthAttachment.format = VK_FORMAT_D32_SFLOAT_S8_UINT;
			depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			
			VkAttachmentReference depthAttachmentRef{};
			depthAttachmentRef.attachment = 1;
			depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			subpassDescription.pDepthStencilAttachment = &depthAttachmentRef;

		}
		// Use subpass dependencies for layout transitions
		auto& dependency = dependencies.emplace_back();
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
			| VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		// Render pass 
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = (uint32_t)attachments.size();
		renderPassInfo.pAttachments = attachments.data();

		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpassDescription;

		renderPassInfo.dependencyCount = (uint32_t)dependencies.size();
		renderPassInfo.pDependencies = dependencies.data();

		PG_VK_ASSERT(vkCreateRenderPass(VulkanContext::GetLogicalDevice()->GetNative(),
			&renderPassInfo, nullptr, &renderPass));

		m_RenderData->Renderpass = CreateRef<RenderPass>(renderPass);
	}
	#pragma endregion

	void Renderer2D::Init()
	{
		m_RenderData = new RenderData;

		auto framesInFlight = RenderManager::GetSettings().FramesInFlight;

		// Inititializes vulkan objects
		VulkanHelpers::AllocatePrimaryCommandBuffers(m_RenderData->RenderToTextureCommandBuffers, framesInFlight);

		Init_Internal();

		// Allocating enough memory to hold specified maximum vertices
		m_RenderData->QuadVertexBuffer = CreateRef<VertexBuffer>(m_RenderData->MaxVertices * sizeof(QuadVertex));
		m_RenderData->QuadVertexBufferBase = new QuadVertex[m_RenderData->MaxVertices];
		m_RenderData->QuadVertexBufferPtr = m_RenderData->QuadVertexBufferBase;

		// Allocating and calculating all indices
		{
			uint32_t* quadIndices = new uint32_t[m_RenderData->MaxIndices];
			uint32_t offset = 0;
			for (uint32_t i = 0; i < m_RenderData->MaxIndices; i += 6)
			{
				quadIndices[i + 0] = offset + 0;
				quadIndices[i + 1] = offset + 1;
				quadIndices[i + 2] = offset + 2;

				quadIndices[i + 3] = offset + 2;
				quadIndices[i + 4] = offset + 3;
				quadIndices[i + 5] = offset + 0;

				offset += 4;
			}
			m_RenderData->QuadIndexBuffer = CreateRef<IndexBuffer>(quadIndices, m_RenderData->MaxIndices);
			delete[] quadIndices;
		}

		uint32_t whiteColor = 0xffffffff - 1;
		m_RenderData->WhiteTexture = CreateRef<Texture2D>(whiteColor);
		m_RenderData->TestTexture2 = CreateRef<Texture2D>("assets/textures/texture.jpg");
		m_RenderData->TestTexture3 = CreateRef<Texture2D>("assets/textures/texture2.jpg");

		m_RenderData->QuadShader = CreateRef<Shader>("assets/shaders/Texture.glsl");

		m_RenderData->QuadShader->AddTextureIntoBinding(1, m_RenderData->WhiteTexture);
		m_RenderData->QuadShader->AddTextureIntoBinding(1, m_RenderData->TestTexture3);

		m_RenderData->QuadShader->GenerateDescriptors();

		m_RenderData->FlatColorPipeline = CreateRef<GraphicsPipeline>();
		m_RenderData->FlatColorPipeline->Invalidate(m_RenderData->Renderpass->GetNative(),
			m_RenderData->QuadShader);
	}
    
	void Renderer2D::Shutdown()
	{
		delete[] m_RenderData->QuadVertexBufferBase;
		delete m_RenderData;
		m_RenderData = nullptr;
	}

	void Renderer2D::BeginScene(const Camera& camera)
	{
		// Camera setup
		m_RenderData->QuadShader->SetUniformData(0, &camera.GetViewProjection());

		StartBatch();
	}

	void Renderer2D::DrawSprite(const TransformComponent& transform, const SpriteRendererComponent& src, int entityID)
	{
		// TODO: texturing
		DrawQuad(transform.GetTransform(), src.Color, 0);
	}

	void Renderer2D::DrawQuad(const glm::mat4& transform, const glm::vec4& color, uint32_t texIndex)
	{
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
			m_RenderData->QuadVertexBufferPtr->Position = transform * quadVertexPositions[i];
			m_RenderData->QuadVertexBufferPtr->Color = color;
			m_RenderData->QuadVertexBufferPtr->TextureCoord = textureCoords[i];
			m_RenderData->QuadVertexBufferPtr->TexIndex = (float)texIndex; // TODO: MAKE THIS INT
			//s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
			//s_Data.QuadVertexBufferPtr->EntityID = entityID;
			m_RenderData->QuadVertexBufferPtr++;
		}

		m_RenderData->QuadIndexCount += 6;

		m_RenderData->Stats.QuadCount++;
	}

	void Renderer2D::EndScene()
	{
		Flush();
	}

	const Renderer2D::Statistics& Renderer2D::GetStats() const
	{
		return m_RenderData->Stats;
	}

	// Only resize when main window resize
	void Renderer2D::OnResize(uint32_t width, uint32_t height)
	{
		ScopedTimer timer("Renderer2d on resize");

		auto device = VulkanContext::GetLogicalDevice()->GetNative();
		{
			auto& depthAttachment = m_RenderData->DepthImage;
			depthAttachment->Invalidate(width, height);
			for (size_t i = 0; i < m_RenderData->Framebuffers.size(); i++)
			{
				auto& colorAttachment = m_RenderData->RenderImages[i];
				colorAttachment->Invalidate(width, height);

				m_RenderData->Framebuffers[i]->Invalidate(width, height, colorAttachment->GetImageView(), m_RenderData->DepthImage->GetImageView());
			}
		}
		for (size_t i = 0; i < m_RenderData->ImGuiTextureDescriptorSets.size(); ++i)
		{
			VkDescriptorImageInfo desc_image[1] = {};
			desc_image[0].sampler = m_RenderData->TextureSampler;
			desc_image[0].imageView = m_RenderData->RenderImages[i]->GetImageView();
			desc_image[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			VkWriteDescriptorSet write_desc[1] = {};
			write_desc[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write_desc[0].dstSet = m_RenderData->ImGuiTextureDescriptorSets[i];
			write_desc[0].descriptorCount = 1;
			write_desc[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			write_desc[0].pImageInfo = desc_image;
			vkUpdateDescriptorSets(device, 1, write_desc, 0, NULL);
		}

		// Re-record command buffer cause framebuffers were invalidated
		RecordCommandBuffer();
	}

	void* Renderer2D::GetTexture()
	{
		const auto swapChain = (Swapchain*)Application::Get()->GetWindow()->GetSwapchain();
		return m_RenderData->ImGuiTextureDescriptorSets[swapChain->GetCurrentFrame()]; // TODO:REMOVE THIS
	}

	glm::vec2 Renderer2D::GetTextureSize()
	{
		const auto swapChain = (Swapchain*)Application::Get()->GetWindow()->GetSwapchain();
		auto& currentImage = m_RenderData->RenderImages[swapChain->GetCurrentFrame()];
		return { currentImage->GetWidth(), currentImage->GetHeight() };
	}

	void Renderer2D::StartBatch()
	{
		m_RenderData->QuadIndexCount = 0;
		m_RenderData->QuadVertexBufferPtr = m_RenderData->QuadVertexBufferBase;

		// Resets stats
		m_RenderData->Stats.QuadCount = 0;
	}

	void Renderer2D::Flush()
	{
		const auto swapChain = (Swapchain*)Application::Get()->GetWindow()->GetSwapchain(); // TODO: remove swapchain everywhere lol
	
		if (m_RenderData->QuadIndexCount) {
			uint32_t dataSize = (uint32_t)((uint8_t*)m_RenderData->QuadVertexBufferPtr - (uint8_t*)m_RenderData->QuadVertexBufferBase);
			m_RenderData->QuadVertexBuffer->SetData(m_RenderData->QuadVertexBufferBase, dataSize, swapChain->GetCurrentFrame());
		}

		RecordCommandBuffer();
	}

	void Renderer2D::RecordCommandBuffer()
	{
		RenderManager::SubmitPrimary([m_RenderData = m_RenderData](uint32_t currentFrame)
			{
				auto currentRenderCmdBuffer = m_RenderData->RenderToTextureCommandBuffers[currentFrame];
				auto currentRenderPass = m_RenderData->Renderpass->GetNative();
				auto currentFramebuffer = m_RenderData->Framebuffers[currentFrame]->GetNative();// Image index used here

				auto indexCount = m_RenderData->QuadIndexCount;
				auto& pipeline = m_RenderData->FlatColorPipeline;
				auto& shader = m_RenderData->QuadShader;

				VkExtent2D extent = { m_RenderData->RenderImages[currentFrame]->GetWidth(),
					m_RenderData->RenderImages[currentFrame]->GetHeight() };

				// CommandBuffers
				// CommandBuffers
				// CommandBuffers
				vkResetCommandBuffer(currentRenderCmdBuffer, 0);

				VkCommandBufferBeginInfo beginInfo{};
				beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
				beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // Optional
				beginInfo.pInheritanceInfo = nullptr; // Optional
				PG_VK_ASSERT(vkBeginCommandBuffer(currentRenderCmdBuffer, &beginInfo));

				// FIXME: Figure out what to do with this
				if(m_RenderData->QuadIndexCount) 
				{
					// Executing secondary command buffers
					auto secCmdBuffers = m_RenderData->QuadVertexBuffer->GetSecondaryCommandBuffer()[currentFrame];
					vkCmdExecuteCommands(currentRenderCmdBuffer, 1, &secCmdBuffers);
				}

				// RenderPass Begin
				// RenderPass Begin
				// RenderPass Begin
				{
					VkRenderPassBeginInfo renderPassInfo{};
					renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
					renderPassInfo.renderPass = currentRenderPass;
					renderPassInfo.framebuffer = currentFramebuffer;
					renderPassInfo.renderArea.offset = { 0, 0 };
					renderPassInfo.renderArea.extent = extent;

					std::array<VkClearValue, 2> clearValues{};
					clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
					clearValues[1].depthStencil = { 1.0f, 0 };

					renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
					renderPassInfo.pClearValues = clearValues.data();
					vkCmdBeginRenderPass(currentRenderCmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

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

					// Quads
					if (m_RenderData->QuadIndexCount) 
					{
						m_RenderData->FlatColorPipeline->Bind(currentRenderCmdBuffer);

						VkBuffer vertexBuffers[]{ m_RenderData->QuadVertexBuffer->GetNative() };
						VkDeviceSize offsets[] = { 0 };
						vkCmdBindVertexBuffers(currentRenderCmdBuffer, 0, 1, vertexBuffers, offsets);
						vkCmdBindIndexBuffer(currentRenderCmdBuffer, m_RenderData->QuadIndexBuffer->GetNative(), 0, VK_INDEX_TYPE_UINT32);

						auto descriptorSets = shader->GetCurrentDescriptorSet();

						vkCmdBindDescriptorSets(currentRenderCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
							pipeline->GetPipelineLayout(), 0, 1, &descriptorSets, 0, nullptr);

						vkCmdDrawIndexed(currentRenderCmdBuffer, indexCount, 1, 0, 0, 0);
					}
					// RenderPass End
					// RenderPass End
					// RenderPass End
					vkCmdEndRenderPass(currentRenderCmdBuffer);

					PG_VK_ASSERT(vkEndCommandBuffer(currentRenderCmdBuffer));
				}
				return currentRenderCmdBuffer;
			});
	}

}