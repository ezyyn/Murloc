#pragma once

#include <vulkan/vulkan.h>

#include "GraphicsPipeline.h"
#include "Shader.h"

namespace PG {

	class CommandQueue;

	class Swapchain {
	public:
		Swapchain();
		~Swapchain();

		// Acquires next image
		void BeginFrame();
		void SwapFrame();

		void Invalidate();

		VkRenderPass GetRenderPass() const { return m_RenderPass; }

		VkExtent2D GetExtent() const { return m_Extent; }

		const std::vector<VkImageView>& GetSwapchainImageView() const { return m_SwapchainImageViews; }

		uint32_t GetCurrentFrame() const { return m_CurrentFrame; }
		uint32_t GetCurrentImageIndex() const { return m_ImageIndex; }

		VkFramebuffer GetCurrentFramebuffer() const { return m_Framebuffers[m_ImageIndex]; }
		VkCommandBuffer GetCurrentMainCommandBuffer() const { return m_MainCommandBuffers[m_CurrentFrame]; }
	private:
		VkSwapchainKHR CreateSwapchain(VkExtent2D extent);
		void CreateRenderPass();
		void CreateImageViews();
		void CreateFramebuffers(uint32_t width, uint32_t height);

		void Submit();
		void Present();
	private:
		uint32_t m_CurrentFrame{ 0 };
		std::vector<VkFramebuffer> m_Framebuffers;
		VkRenderPass m_RenderPass;

		std::vector<VkSemaphore> m_RenderFinishedSemaphores;
		std::vector<VkSemaphore> m_PresentSemaphores;
		std::vector<VkFence> m_InFlightFences;

		std::vector<VkCommandBuffer> m_MainCommandBuffers;

		std::vector<VkImage> m_SwapchainImages;
		std::vector<VkImageView> m_SwapchainImageViews;
		uint32_t m_ImageIndex{ 0 };

		VkFormat m_SwapChainFormat;

		VkExtent2D m_Extent;
		VkSwapchainKHR m_NativeSwapchain{ VK_NULL_HANDLE };
	};

}