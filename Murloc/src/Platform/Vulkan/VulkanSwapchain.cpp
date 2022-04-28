#include "murpch.hpp"

#include "VulkanSwapchain.hpp"
#include "Vulkan.hpp"
#include "VulkanRenderer.hpp" 

namespace Murloc {

	VulkanSwapchain::VulkanSwapchain(VkRenderPass renderPass)
	{
		const auto& framesInFlight = VulkanRenderer::FramesInFlight();

		auto& physicalDeviceSupp = Vulkan::GetDevice()->GetPhysicalDevice()->GetSupportDetails();

		m_SwapchainSupportDetails.Capabilities = physicalDeviceSupp.Capabilities;
		m_SwapchainSupportDetails.Extent = physicalDeviceSupp.Extent;
		m_SwapchainSupportDetails.MinImageCount = physicalDeviceSupp.MinImageCount;
		m_SwapchainSupportDetails.PresentMode = physicalDeviceSupp.PresentMode;
		m_SwapchainSupportDetails.SurfaceFormat = physicalDeviceSupp.SurfaceFormat;

		CleanupAndRecreate(m_SwapchainSupportDetails.Extent.width, m_SwapchainSupportDetails.Extent.height, renderPass);

		// Command Buffers
		m_RenderCommandBuffers.resize(framesInFlight);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = Vulkan::GetDevice()->GetCommandPool();
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)m_RenderCommandBuffers.size();

		MUR_VK_ASSERT(vkAllocateCommandBuffers(Vulkan::GetDevice()->GetNative(), &allocInfo, m_RenderCommandBuffers.data()));

		// Fences and semaphores
		m_ImageAvailableSemaphores.resize(framesInFlight);
		m_RenderFinishedSemaphores.resize(framesInFlight);
		m_InFlightFences.resize(framesInFlight);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (uint32_t i = 0; i < framesInFlight; ++i)
		{
			MUR_VK_ASSERT(vkCreateSemaphore(Vulkan::GetDevice()->GetNative(), &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[i]));
			MUR_VK_ASSERT(vkCreateSemaphore(Vulkan::GetDevice()->GetNative(), &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i]));
			MUR_VK_ASSERT(vkCreateFence(Vulkan::GetDevice()->GetNative(), &fenceInfo, nullptr, &m_InFlightFences[i]));
		}
	}

	VulkanSwapchain::~VulkanSwapchain()
	{
		auto device = Vulkan::GetDevice()->GetNative();

		for (size_t i = 0; i < VulkanRenderer::FramesInFlight(); i++) {
			vkDestroySemaphore(device, m_RenderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(device, m_ImageAvailableSemaphores[i], nullptr);
			vkDestroyFence(device, m_InFlightFences[i], nullptr);
		}

		for (auto framebuffer : m_Framebuffers) {
			vkDestroyFramebuffer(device, framebuffer, nullptr);
		}

		for (auto imageView : m_SwapchainImageViews) {
			vkDestroyImageView(device, imageView, nullptr);
		}

		vkDestroySwapchainKHR(device, m_Swapchain, nullptr);
	}

	bool VulkanSwapchain::NewFrame()
	{
		auto fence = &m_InFlightFences[m_CurrentFrame];

		vkWaitForFences(Vulkan::GetDevice()->GetNative(), 1, fence, VK_TRUE, UINT64_MAX);
		VkResult result = vkAcquireNextImageKHR(Vulkan::GetDevice()->GetNative(), m_Swapchain, UINT64_MAX, m_ImageAvailableSemaphores[m_CurrentFrame], VK_NULL_HANDLE, &m_ImageIndex);


		if (result == VK_ERROR_OUT_OF_DATE_KHR /*|| result == VK_SUBOPTIMAL_KHR*/) {
			return true;
		}

		vkResetFences(Vulkan::GetDevice()->GetNative(), 1, fence);

		return false;
	}

	bool VulkanSwapchain::EndFrame()
	{
		bool rebuild = false;
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		VkSemaphore renderFinished[] = { m_RenderFinishedSemaphores[m_CurrentFrame] };
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = renderFinished;

		VkSwapchainKHR swapChains[] = { m_Swapchain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &m_ImageIndex;
		presentInfo.pResults = nullptr; // Optional

		VkResult result = vkQueuePresentKHR(Vulkan::GetDevice()->GetPresentQueue(), &presentInfo);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
			rebuild = true;
		} else {
			MUR_CORE_ASSERT(result == VK_SUCCESS, result);
		}

		m_CurrentFrame = (m_CurrentFrame + 1) % (uint32_t)m_RenderCommandBuffers.size();
		return rebuild;
	}

	void VulkanSwapchain::CleanupAndRecreate(uint32_t width, uint32_t height, VkRenderPass renderpass)
	{
		auto device = Vulkan::GetDevice()->GetNative();
		if (m_Swapchain) {

			for (auto framebuffer : m_Framebuffers) {
				vkDestroyFramebuffer(device, framebuffer, nullptr);
			}

			for (auto imageView : m_SwapchainImageViews) {
				vkDestroyImageView(device, imageView, nullptr);
			}
		}

		// Create new swapchain width old swapchain
		VkSwapchainKHR newSwapchain = CreateSwapchain(width, height);
		// destroy old swapchain if exist
		if (m_Swapchain) {
			/*vkFreeCommandBuffers(Vulkan::GetDevice()->GetNative(), Vulkan::GetDevice()->GetCommandPool(), 
				(uint32_t)m_RenderCommandBuffers.size(), m_RenderCommandBuffers.data());

			VkCommandBufferAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.commandPool = Vulkan::GetDevice()->GetCommandPool();
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandBufferCount = (uint32_t)m_RenderCommandBuffers.size();
			MUR_VK_ASSERT(vkAllocateCommandBuffers(Vulkan::GetDevice()->GetNative(), &allocInfo, m_RenderCommandBuffers.data()));*/

			vkDestroySwapchainKHR(device, m_Swapchain, nullptr);
			m_Swapchain = VK_NULL_HANDLE;
		}
		// Assign new swapchain
		m_Swapchain = newSwapchain;

		CreateImageViews();
		CreateFramebuffers(width, height, renderpass);
	}

	void VulkanSwapchain::BeginRecordCmdBuffer()
	{
		vkResetCommandBuffer(m_RenderCommandBuffers[m_CurrentFrame], 0);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0; // Optional
		beginInfo.pInheritanceInfo = nullptr; // Optional

		MUR_VK_ASSERT(vkBeginCommandBuffer(m_RenderCommandBuffers[m_CurrentFrame], &beginInfo));
	}

	void VulkanSwapchain::EndRecordCmdBuffer()
	{
		MUR_VK_ASSERT(vkEndCommandBuffer(m_RenderCommandBuffers[m_CurrentFrame]));
	}

	void VulkanSwapchain::Submit()
	{
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		VkSemaphore waitSemaphores[] = { m_ImageAvailableSemaphores[m_CurrentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		VkCommandBuffer cmdBuffers[] = { m_RenderCommandBuffers[m_CurrentFrame]};

		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = cmdBuffers;
		VkSemaphore signalSemaphores[] = { m_RenderFinishedSemaphores[m_CurrentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		MUR_VK_ASSERT(vkQueueSubmit(Vulkan::GetDevice()->GetGraphicsQueue(), 1, &submitInfo, m_InFlightFences[m_CurrentFrame]));
	}

	VkSwapchainKHR VulkanSwapchain::CreateSwapchain(uint32_t width, uint32_t height)
	{
		VkSwapchainKHR newSwapchain{ VK_NULL_HANDLE };

		m_SwapChainExtent = { width, height };

		auto& indices = Vulkan::GetDevice()->GetPhysicalDevice()->GetQueueFamilyIndices();

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = Vulkan::GetSurface();
		createInfo.minImageCount = m_SwapchainSupportDetails.MinImageCount;
		createInfo.imageFormat = m_SwapchainSupportDetails.SurfaceFormat.format;
		createInfo.imageColorSpace = m_SwapchainSupportDetails.SurfaceFormat.colorSpace;
		createInfo.imageExtent = m_SwapChainExtent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		createInfo.preTransform = m_SwapchainSupportDetails.Capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR; // V-Sync
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = m_Swapchain; // Using old swapchain

		uint32_t queueFamilyIndices[] = { indices.GraphicsFamily.value(), indices.PresentFamily.value() };

		if (indices.GraphicsFamily != indices.PresentFamily) {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			//createInfo.queueFamilyIndexCount = 0; // Optional
			//createInfo.pQueueFamilyIndices = nullptr; // Optional
		}

		MUR_VK_ASSERT(vkCreateSwapchainKHR(Vulkan::GetDevice()->GetNative(), &createInfo, nullptr, &newSwapchain));
		uint32_t imageCount;
		MUR_VK_ASSERT(vkGetSwapchainImagesKHR(Vulkan::GetDevice()->GetNative(), newSwapchain, &imageCount, nullptr));
		m_SwapchainImages.resize(imageCount);
		MUR_VK_ASSERT(vkGetSwapchainImagesKHR(Vulkan::GetDevice()->GetNative(), newSwapchain, &imageCount, m_SwapchainImages.data()));

		m_SwapchainImageFormat = m_SwapchainSupportDetails.SurfaceFormat.format;

		return newSwapchain;
	}

	void VulkanSwapchain::CreateImageViews()
	{
		m_SwapchainImageViews.resize(m_SwapchainImages.size());

		for (size_t i = 0; i < m_SwapchainImages.size(); ++i)
		{
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = m_SwapchainImages[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = m_SwapchainImageFormat;
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;
			MUR_VK_ASSERT(vkCreateImageView(Vulkan::GetDevice()->GetNative(), &createInfo, nullptr, &m_SwapchainImageViews[i]));
		}
	}

	void VulkanSwapchain::CreateFramebuffers(uint32_t width, uint32_t height, VkRenderPass renderpass)
	{
		m_Framebuffers.resize(m_SwapchainImageViews.size());
		for (size_t i = 0; i < m_SwapchainImageViews.size(); i++) {

			VkImageView attachments[] = {
				m_SwapchainImageViews[i] // Color buffer ?
			};

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderpass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = width;
			framebufferInfo.height = height;
			framebufferInfo.layers = 1;

			MUR_VK_ASSERT(vkCreateFramebuffer(Vulkan::GetDevice()->GetNative(), &framebufferInfo, nullptr, &m_Framebuffers[i]));
		}
	}

}

