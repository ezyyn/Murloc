#include "murpch.hpp"

#include "Murloc/Renderer/Renderer.hpp"

#include "Platform/Vulkan/VulkanRenderer.hpp"

#include "Platform/Vulkan/VulkanShader.hpp"

// BatchRenderer for now 

namespace Murloc {

	struct RenderData
	{
		Ref<VulkanShader> FlatColorShader;
		Ref<VulkanVertexBuffer> QuadVertexBuffer;

	};

	static RenderData s_Data;

	void Renderer::Init()
	{
		VulkanRenderer::Init();

		const std::vector<QuadVertex> vertices = {
			{ { 0.0f,-0.5f, 0.0f}, { 1.0f, 0.0f, 0.0f, 1.0f} },
			{ { 0.5f, 0.5f, 0.0f}, { 0.0f, 1.0f, 0.0f, 1.0f} },
			{ {-0.5f, 0.5f, 0.0f}, { 0.0f, 0.0f, 1.0f, 1.0f} },
		};

		s_Data.QuadVertexBuffer = CreateRef<VulkanVertexBuffer>(vertices.size() * sizeof(QuadVertex));
		s_Data.QuadVertexBuffer->SetData((void*)vertices.data());


		//s_Data.FlatColorShader = CreateRef<VulkanShader>("assets/shaders/FlatColor.glsl");
	}

	void Renderer::BeginFrame()
	{
		VulkanRenderer::Begin();
	}

	void Renderer::EndFrame()
	{
		VulkanRenderer::Render(s_Data.QuadVertexBuffer, 3);
	}

	void Renderer::Shutdown()
	{
		VulkanRenderer::WaitIdle();

		s_Data.QuadVertexBuffer.reset();
		VulkanRenderer::Shutdown();
	}

	void Renderer::OnResize(uint32_t width, uint32_t height)
	{
		VulkanRenderer::OnResize(width, height);
	}

}