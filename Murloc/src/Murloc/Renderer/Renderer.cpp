#include "murpch.hpp"

#include "Murloc/Renderer/Renderer.hpp"

#include "Platform/Vulkan/VulkanRenderer.hpp"

#include "Platform/Vulkan/VulkanShader.hpp"

#include "Murloc/ImGui/ImGuiLayer.hpp"

#include <imgui.h>

// BatchRenderer

namespace Murloc {

	struct RenderData
	{
		Ref<VulkanShader> FlatColorShader;
		Ref<VulkanVertexBuffer> QuadVertexBuffer;
		Ref<VulkanIndexBuffer> QuadIndexBuffer;
	};

	static RenderData s_Data;

	void Renderer::Init()
	{
		VulkanRenderer::Init();

		const std::vector<QuadVertex> vertices = {
			{ {-0.5f,-0.5f, 0.0f}, { 1.0f, 0.0f, 0.0f, 1.0f} },
			{ { 0.5f, -0.5f, 0.0f}, { 1.0f, 1.0f, 1.0f, 1.0f} },
			{ { 0.5f, 0.5f, 0.0f}, { 0.0f, 1.0f, 0.0f, 1.0f} },
			{ { -0.5f, 0.5f, 0.0f}, { 0.0f, 1.0f, 0.0f, 1.0f} },
		};

		const std::vector<uint32_t> indices = {
			0, 1, 2, 2, 3, 0
		};

		s_Data.QuadIndexBuffer = CreateRef<VulkanIndexBuffer>(indices.data(), 6);

		s_Data.QuadVertexBuffer = CreateRef<VulkanVertexBuffer>(vertices.size() * sizeof(QuadVertex));

		s_Data.QuadVertexBuffer->SetData((void*)vertices.data());

		//s_Data.FlatColorShader = CreateRef<VulkanShader>("assets/shaders/FlatColor.glsl");
		/*layer = new ImGuiLayer();
		layer->OnAttach();*/
	}

	void Renderer::BeginFrame()
	{
		// Resets buffer pointer
	}


	void Renderer::EndFrame()
	{
		VulkanRenderer::DrawIndexed(s_Data.QuadVertexBuffer, s_Data.QuadIndexBuffer, 6);
	}

	void Renderer::Shutdown()
	{
		VulkanRenderer::WaitIdle();

		s_Data.QuadIndexBuffer.reset();
		s_Data.QuadVertexBuffer.reset();
		VulkanRenderer::Shutdown();
	}

	void Renderer::OnResize(uint32_t width, uint32_t height)
	{
		VulkanRenderer::RecreateGraphicsPipeline();
	}

}