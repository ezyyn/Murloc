#pragma once

#include <vulkan/vulkan.h>

#include "GraphicsPipeline.h"
#include "Shader.h"

namespace Murloc {

	class CommandQueue;

	class Swapchain {
	public:
		Swapchain();
		~Swapchain();

		// Acquires next image
		void BeginFrame();
		void SwapFrame();

		VkSwapchainKHR CreateSwapchain(VkExtent2D extent);

		static inline VkFramebuffer s_CurrentFramebuffer;
		static inline VkCommandBuffer s_CurrentRenderCommandbuffer;
		static inline VkRenderPass s_RenderPass{ VK_NULL_HANDLE };

		void Invalidate();

		VkExtent2D GetExtent() const { return m_Extent; }

		static inline uint32_t s_CurrentFrame{ 0 };
		uint32_t m_ImageIndex{ 0 };
	private:
		void CreateImageViews();
		void CreateFramebuffers(uint32_t width, uint32_t height);
		void CreateRenderPass();

		void Submit();
		void Present();
	private:
		std::vector<VkFramebuffer> m_Framebuffers;
		std::vector<VkCommandBuffer> m_RenderCommandBuffers;

		std::vector<VkSemaphore> m_RenderFinishedSemaphores;
		std::vector<VkSemaphore> m_ImageAvailableSemaphores;
		std::vector<VkFence> m_InFlightFences;

		std::vector<VkImage> m_SwapchainImages;
		std::vector<VkImageView> m_SwapchainImageViews;

		VkExtent2D m_Extent;
		VkSwapchainKHR m_NativeSwapchain{ VK_NULL_HANDLE };
	};

}