#pragma once

#include <vulkan/vulkan.h>

namespace Murloc {

	class VulkanSwapchain {
	public:
		VulkanSwapchain(VkRenderPass renderPass);
		~VulkanSwapchain();

		bool NewFrame();
		bool EndFrame();

		void CleanupAndRecreate(uint32_t width, uint32_t height, VkRenderPass renderpass);

		const std::vector<VkImageView>& GetImageViews() const { return m_SwapchainImageViews; }

		VkExtent2D GetSwapChainExtent() const { return m_SwapChainExtent; }

		VkFormat GetSwapchainImageFormat() const {
			return m_SwapchainImageFormat;
		}

		VkSwapchainKHR GetNative() const { return m_Swapchain; };

		VkFramebuffer GetCurrentSwapchainFramebuffer() const { return m_Framebuffers[m_ImageIndex]; }

		VkCommandBuffer GetCurrentRenderCmdBuffer() const { return m_RenderCommandBuffers[m_CurrentFrame]; }

		template<typename Func>
		void RecordCmdBuffer(Func&& func)
		{
			BeginRecordCmdBuffer();
			func(m_RenderCommandBuffers[m_CurrentFrame]);
			EndRecordCmdBuffer();
		}

		void Submit();
	private:
		void BeginRecordCmdBuffer();
		void EndRecordCmdBuffer();

		VkSwapchainKHR CreateSwapchain(uint32_t width, uint32_t height);
		void CreateImageViews();
		void CreateFramebuffers(uint32_t width, uint32_t height, VkRenderPass renderpass);
	private:
		uint32_t m_ImageIndex{ 0 };
		uint32_t m_CurrentFrame{ 0 };

		std::vector<VkFramebuffer> m_Framebuffers;
		std::vector<VkCommandBuffer> m_RenderCommandBuffers;

		std::vector<VkSemaphore> m_RenderFinishedSemaphores;
		std::vector<VkSemaphore> m_ImageAvailableSemaphores;
		std::vector<VkFence> m_InFlightFences;

		VkFormat m_SwapchainImageFormat;
		VkSwapchainKHR m_Swapchain{ VK_NULL_HANDLE };

		VkExtent2D m_SwapChainExtent;

		struct SupportDetails {
			uint32_t MinImageCount;
			VkSurfaceFormatKHR SurfaceFormat;
			VkPresentModeKHR PresentMode;
			VkSurfaceCapabilitiesKHR Capabilities;
			VkExtent2D Extent;
		};

		SupportDetails m_SwapchainSupportDetails;

		std::vector<VkImage> m_SwapchainImages;
		std::vector<VkImageView> m_SwapchainImageViews;
	};

}