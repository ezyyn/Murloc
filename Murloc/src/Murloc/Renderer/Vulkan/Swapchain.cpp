#include "murpch.hpp"

#include "Swapchain.h"

#include "Murloc/Renderer/Vulkan/VulkanContext.h"
#include "Murloc/Renderer/Renderer.hpp"

#include "Murloc/Core/Application.hpp"

// Temp
#include "Murloc/Renderer/Vulkan/IndexBuffer.h" 
#include "Murloc/Renderer/Vulkan/VertexBuffer.h"
#include "Murloc/Renderer/RenderCommand.h"

#include "Murloc/Renderer/CommandQueue.h"
#include "Murloc/Renderer/Vulkan/VulkanUtils.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace Murloc {

	Swapchain::Swapchain()
	{
		const auto& framesInFlight = Renderer::GetSettings().FramesInFlight;
		auto device = VulkanContext::GetLogicalDevice()->GetNative();

		CreateRenderPass();
		Invalidate();

		// Command Buffers
		m_RenderCommandBuffers.resize(framesInFlight);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = VulkanContext::GetLogicalDevice()->GetCommandPool();
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)m_RenderCommandBuffers.size();

		MUR_VK_ASSERT(vkAllocateCommandBuffers(VulkanContext::GetLogicalDevice()->GetNative(), 
			&allocInfo, m_RenderCommandBuffers.data()));

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
			MUR_VK_ASSERT(vkCreateSemaphore(VulkanContext::GetLogicalDevice()->GetNative(), &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[i]));
			MUR_VK_ASSERT(vkCreateSemaphore(VulkanContext::GetLogicalDevice()->GetNative(), &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i]));
			MUR_VK_ASSERT(vkCreateFence(VulkanContext::GetLogicalDevice()->GetNative(), &fenceInfo, nullptr, &m_InFlightFences[i]));
		}

		auto& resourceFreeQueue = VulkanContext::GetContextResourceFreeQueue();

		resourceFreeQueue.PushBack(SYNC_OBJECTS, [device, framesInFlight,
			&m_RenderFinishedSemaphores = m_RenderFinishedSemaphores, 
			&m_ImageAvailableSemaphores = m_ImageAvailableSemaphores, &m_InFlightFences = m_InFlightFences]()
			{
				for (size_t i = 0; i < framesInFlight; i++) {
					vkDestroySemaphore(device, m_RenderFinishedSemaphores[i], nullptr);
					vkDestroySemaphore(device, m_ImageAvailableSemaphores[i], nullptr);
					vkDestroyFence(device, m_InFlightFences[i], nullptr);
				}
			});
		
		resourceFreeQueue.PushBack(FRAMEBUFFERS, [device, &m_Framebuffers = m_Framebuffers]()
			{
				for (auto framebuffer : m_Framebuffers) {
					vkDestroyFramebuffer(device, framebuffer, nullptr);
				}
			});

		resourceFreeQueue.PushBack(IMAGEVIEWS, [device, &m_SwapchainImageViews = m_SwapchainImageViews]()
			{
				for (auto imageView : m_SwapchainImageViews) {
					vkDestroyImageView(device, imageView, nullptr);
				}
			});

		resourceFreeQueue.PushBack(SWAPCHAIN,[device, &m_NativeSwapchain = m_NativeSwapchain]()
			{
				vkDestroySwapchainKHR(device, m_NativeSwapchain, nullptr);
			});

		resourceFreeQueue.PushBack(RENDERPASS, [device, renderPass = s_RenderPass]()
			{
				vkDestroyRenderPass(device, renderPass, nullptr);
			});

		s_CurrentRenderCommandbuffer = m_RenderCommandBuffers[s_CurrentFrame];
		s_CurrentFramebuffer = m_Framebuffers[m_ImageIndex];
	}

	Swapchain::~Swapchain()
	{
	}

	void Swapchain::BeginFrame()
	{
		auto fence = &m_InFlightFences[s_CurrentFrame];
		auto logicalDevice = VulkanContext::GetLogicalDevice()->GetNative();

		// Wait for the previous frame to finish - blocks cpu until signaled
		vkWaitForFences(VulkanContext::GetLogicalDevice()->GetNative(), 1, fence, VK_TRUE, UINT64_MAX);

		VkResult result = vkAcquireNextImageKHR(logicalDevice, m_NativeSwapchain, UINT64_MAX, m_ImageAvailableSemaphores[s_CurrentFrame], VK_NULL_HANDLE, &m_ImageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
			//return true;

			// Test if dynamic pipeline is getting recreated
			MUR_CORE_ASSERT(false); // recreate swapchain
			//Invalidate();
		}

		// set fence back to being unsignaled
		vkResetFences(logicalDevice, 1, fence);

		s_CurrentRenderCommandbuffer = m_RenderCommandBuffers[s_CurrentFrame];
		s_CurrentFramebuffer = m_Framebuffers[m_ImageIndex];
	}

	void Swapchain::SwapFrame()
	{
		auto currentCommandBuffer = m_RenderCommandBuffers[s_CurrentFrame];
		auto currentFramebuffer = m_Framebuffers[m_ImageIndex];

	/*	while (m_RenderCommandQueue.size()) {
			m_RenderCommandQueue.front()(currentCommandBuffer, currentFramebuffer);
		}*/

		Submit();
		Present();
	}

	void Swapchain::Submit()
	{
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		VkSemaphore waitSemaphores[] = { m_ImageAvailableSemaphores[s_CurrentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		VkCommandBuffer cmdBuffers[] = { m_RenderCommandBuffers[s_CurrentFrame] };

		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = cmdBuffers;
		VkSemaphore signalSemaphores[] = { m_RenderFinishedSemaphores[s_CurrentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		MUR_VK_ASSERT(vkQueueSubmit(VulkanContext::GetLogicalDevice()->GetGraphicsQueue(), 1, &submitInfo, m_InFlightFences[s_CurrentFrame]));
	}

	void Swapchain::Present()
	{
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		VkSemaphore renderFinished[] = { m_RenderFinishedSemaphores[s_CurrentFrame] };
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = renderFinished;

		VkSwapchainKHR swapChains[] = { m_NativeSwapchain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &m_ImageIndex;
		presentInfo.pResults = nullptr; // Optional

		VkResult result = vkQueuePresentKHR(VulkanContext::GetLogicalDevice()->GetPresentQueue(), &presentInfo);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {

			//Invalidate();
			//RenderCommand::WaitIdle();
		}

		MUR_VK_ASSERT(result);

		// Swapping
		// Swapping
		// Swapping
		s_CurrentFrame = (s_CurrentFrame + 1) % (uint32_t)m_RenderCommandBuffers.size();
	}

	VkSwapchainKHR Swapchain::CreateSwapchain(VkExtent2D extent)
	{
		auto& physicalDeviceSupp = VulkanContext::GetLogicalDevice()->GetPhysicalDevice().GetSupportDetails();

		VkSwapchainKHR newSwapchain{ VK_NULL_HANDLE };

		m_Extent = extent;

		auto& indices = VulkanContext::GetLogicalDevice()->GetPhysicalDevice().GetQueueFamilyIndices();

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = VulkanContext::GetSurface();
		createInfo.minImageCount = physicalDeviceSupp.MinImageCount;
		createInfo.imageFormat = physicalDeviceSupp.SurfaceFormat.format;
		createInfo.imageColorSpace = physicalDeviceSupp.SurfaceFormat.colorSpace;
		createInfo.imageExtent = m_Extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		createInfo.preTransform = physicalDeviceSupp.Capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR; // V-Sync
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = m_NativeSwapchain; // Using old swapchain

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

		MUR_VK_ASSERT(vkCreateSwapchainKHR(VulkanContext::GetLogicalDevice()->GetNative(), &createInfo, nullptr, &newSwapchain));
		uint32_t imageCount;
		MUR_VK_ASSERT(vkGetSwapchainImagesKHR(VulkanContext::GetLogicalDevice()->GetNative(), newSwapchain, &imageCount, nullptr));
		m_SwapchainImages.resize(imageCount);
		MUR_VK_ASSERT(vkGetSwapchainImagesKHR(VulkanContext::GetLogicalDevice()->GetNative(), newSwapchain, &imageCount, 
			m_SwapchainImages.data()));

		return newSwapchain;
	}

	// Cleans up and create new Swapchain
	void Swapchain::Invalidate()
	{
		RenderCommand::WaitIdle();

		auto device = VulkanContext::GetLogicalDevice()->GetNative();

		VkExtent2D extent = { Application::Get()->GetWindow()->GetWidth(), Application::Get()->GetWindow()->GetHeight() };

		MUR_CORE_WARN("RECREATING SWAPCHAIN {0}, {1}", extent.width, extent.height);

		// Create new swapchain with old swapchain
		VkSwapchainKHR newSwapchain = CreateSwapchain(extent);
		// destroy old swapchain if exist
		if (m_NativeSwapchain) 
		{
			for (auto framebuffer : m_Framebuffers) {
				vkDestroyFramebuffer(device, framebuffer, nullptr);
			}

			for (auto imageView : m_SwapchainImageViews) {
				vkDestroyImageView(device, imageView, nullptr);
			}

			vkDestroySwapchainKHR(device, m_NativeSwapchain, nullptr);
			m_NativeSwapchain = VK_NULL_HANDLE;
		}
		// Assign new swapchain
		m_NativeSwapchain = newSwapchain;

		CreateImageViews();
		CreateFramebuffers(extent.width, extent.height);
	}

	void Swapchain::CreateImageViews()
	{
		auto& physicalDeviceSupp = VulkanContext::GetLogicalDevice()->GetPhysicalDevice().GetSupportDetails();

		m_SwapchainImageViews.resize(m_SwapchainImages.size());

		for (size_t i = 0; i < m_SwapchainImages.size(); ++i)
		{
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = m_SwapchainImages[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = physicalDeviceSupp.SurfaceFormat.format;
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;
			MUR_VK_ASSERT(vkCreateImageView(VulkanContext::GetLogicalDevice()->GetNative(), &createInfo, nullptr, &m_SwapchainImageViews[i]));
		}
	}

	void Swapchain::CreateFramebuffers(uint32_t width, uint32_t height)
	{
		m_Framebuffers.resize(m_SwapchainImageViews.size());
		for (size_t i = 0; i < m_SwapchainImageViews.size(); i++) {

			VkImageView attachments[] = {
				m_SwapchainImageViews[i] // Color buffer ?
			};

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = s_RenderPass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = width;
			framebufferInfo.height = height;
			framebufferInfo.layers = 1;

			MUR_VK_ASSERT(vkCreateFramebuffer(VulkanContext::GetLogicalDevice()->GetNative(), &framebufferInfo, nullptr, &m_Framebuffers[i]));
		}
	}

	void Swapchain::CreateRenderPass()
	{
		auto& physicalDeviceSupp = VulkanContext::GetLogicalDevice()->GetPhysicalDevice().GetSupportDetails();

		// Attachment description
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = physicalDeviceSupp.SurfaceFormat.format;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		// Subpasses and attachment references
		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		// Render pass 
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		MUR_VK_ASSERT(vkCreateRenderPass(VulkanContext::GetLogicalDevice()->GetNative(), 
			&renderPassInfo, nullptr, &s_RenderPass));
	}

}