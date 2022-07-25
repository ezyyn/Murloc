#include "pgpch.h"

#include "Swapchain.h"

#include "Pangolin/Renderer/Vulkan/VulkanContext.h"

#include "Pangolin/Core/Application.h"

// Temp
#include "Pangolin/Renderer/Vulkan/IndexBuffer.h" 
#include "Pangolin/Renderer/Vulkan/VertexBuffer.h"

#include "Pangolin/Renderer/CommandQueue.h"
#include "Pangolin/Renderer/Vulkan/VulkanUtils.h"

#include "Pangolin/ImGui/ImGuiLayer.h"

#define PG_VULKAN_FUNCTIONS
#include "Pangolin/Renderer/RenderManager.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <Pangolin/Core/Timer.h>

namespace PG {

	Swapchain::Swapchain()
	{
		const auto& framesInFlight = RenderManager::GetSettings().FramesInFlight;
		m_SwapChainFormat = VK_FORMAT_B8G8R8A8_UNORM;
		CreateRenderPass();

		// Creates:
		//			- Swapchain
		//			- Image views from retrieved swapchain images
		//			- Framebuffers
		Invalidate();

		// Allocating main render command buffers
		VulkanHelpers::AllocatePrimaryCommandBuffers(m_MainCommandBuffers, framesInFlight);

		// Fences and semaphores
		VulkanHelpers::CreateSemaphores(m_PresentSemaphores, framesInFlight);
		VulkanHelpers::CreateSemaphores(m_RenderFinishedSemaphores, framesInFlight);
		VulkanHelpers::CreateFences(m_InFlightFences, framesInFlight, VK_FENCE_CREATE_SIGNALED_BIT);

		// Resource Free Queue
		auto& resourceFreeQueue = VulkanContext::GetContextResourceFreeQueue();
		auto device = VulkanContext::GetLogicalDevice()->GetNative();

		resourceFreeQueue.PushBack(FRAMEBUFFER, [device, &m_Framebuffers = m_Framebuffers]()
			{
				for (auto framebuffer : m_Framebuffers) {
					vkDestroyFramebuffer(device, framebuffer, nullptr);
				}
			});

		resourceFreeQueue.PushBack(IMAGEVIEW, [device, &m_SwapchainImageViews = m_SwapchainImageViews]()
			{
				for (auto imageView : m_SwapchainImageViews) {
					vkDestroyImageView(device, imageView, nullptr);
				}
			});

		resourceFreeQueue.PushBack(SWAPCHAIN,[device, &m_NativeSwapchain = m_NativeSwapchain]()
			{
				vkDestroySwapchainKHR(device, m_NativeSwapchain, nullptr);
			});
	}

	Swapchain::~Swapchain()
	{
	}

	void Swapchain::BeginFrame()
	{
		auto device = VulkanContext::GetLogicalDevice()->GetNative();

		// No need to check if this function returns VK_ERROR_OUT_OF_DATE_KHR or VK_SUBOPTIMAL_KHR.
		// Cause swapchain is recreated before this is called.
		PG_VK_ASSERT(vkAcquireNextImageKHR(device, m_NativeSwapchain, UINT64_MAX, m_PresentSemaphores[m_CurrentFrame], VK_NULL_HANDLE, &m_ImageIndex));
	}

	void Swapchain::SwapFrame()
	{
		Submit();
		Present();
	}
	void Swapchain::Submit()
	{
		auto device = VulkanContext::GetLogicalDevice()->GetNative();

		// Retrives all current commandBuffers and submit them to queue
		auto& submitCommandBuffers = RenderManager::DrawData();

		// Adds main command buffer(ImGui) as last
		submitCommandBuffers.push_back(m_MainCommandBuffers[m_CurrentFrame]);

		VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &m_PresentSemaphores[m_CurrentFrame];
		submitInfo.pWaitDstStageMask = &waitStage;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &m_RenderFinishedSemaphores[m_CurrentFrame];
		submitInfo.commandBufferCount = (uint32_t)submitCommandBuffers.size();
		submitInfo.pCommandBuffers = submitCommandBuffers.data();

		// Set fence back to being unsignaled
		PG_VK_ASSERT(vkResetFences(device, 1, &m_InFlightFences[m_CurrentFrame]));
		PG_VK_ASSERT(vkQueueSubmit(VulkanContext::GetLogicalDevice()->GetGraphicsQueue(), 1, &submitInfo, m_InFlightFences[m_CurrentFrame]));
	}

	void Swapchain::Present()
	{
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		// Wait for the frame to render
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &m_RenderFinishedSemaphores[m_CurrentFrame];
		// Specify swapchain
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &m_NativeSwapchain;

		// Image index
		presentInfo.pImageIndices = &m_ImageIndex;

		// No need to check if this function returns VK_ERROR_OUT_OF_DATE_KHR or VK_SUBOPTIMAL_KHR.
		VkResult result = vkQueuePresentKHR(VulkanContext::GetLogicalDevice()->GetPresentQueue(), &presentInfo);
		if (result != VK_SUCCESS) 
		{
			if (result == VK_ERROR_OUT_OF_DATE_KHR)
			{
				Invalidate();
				return;
			}

			// Something went wrong
			PG_CORE_ASSERT(false);
		}
		// Wait for the previous frame to finish - blocks cpu until signaled
		PG_VK_ASSERT(vkWaitForFences(VulkanContext::GetLogicalDevice()->GetNative(), 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX));

		// Cycle frames in flights
		m_CurrentFrame = (m_CurrentFrame + 1) % RenderManager::GetSettings().FramesInFlight;
	}

	// Cleans up and create new Swapchain
	void Swapchain::Invalidate()
	{
		auto device = VulkanContext::GetLogicalDevice()->GetNative();

		vkDeviceWaitIdle(device);

		VkExtent2D extent = { Application::Get()->GetWindow()->GetWidth(), Application::Get()->GetWindow()->GetHeight() };

		PG_CORE_WARN("RECREATING SWAPCHAIN {0}, {1}", extent.width, extent.height);

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

		// Creates image views
		CreateImageViews();

		// Creates framebuffers
		CreateFramebuffers(extent.width, extent.height);

		vkDeviceWaitIdle(device);
	}

	VkSwapchainKHR Swapchain::CreateSwapchain(VkExtent2D extent)
	{
		m_Extent = extent;

		auto& physicalDeviceSupp = VulkanContext::GetLogicalDevice()->GetPhysicalDevice().GetSupportDetails();

		VkSwapchainKHR newSwapchain{ VK_NULL_HANDLE };

		auto& indices = VulkanContext::GetLogicalDevice()->GetPhysicalDevice().GetQueueFamilyIndices();

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = VulkanContext::GetSurface();
		createInfo.minImageCount = physicalDeviceSupp.MinImageCount;
		createInfo.imageFormat = m_SwapChainFormat;
		createInfo.imageColorSpace = physicalDeviceSupp.SurfaceFormat.colorSpace;
		createInfo.imageExtent = m_Extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		createInfo.preTransform = physicalDeviceSupp.Capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR; // V-Sync
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = m_NativeSwapchain; // Using old swapchain

		if (indices.GraphicsFamily != indices.PresentFamily) 
		{
			uint32_t queueFamilyIndices[] = { indices.GraphicsFamily.value(), indices.PresentFamily.value() };
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}

		PG_VK_ASSERT(vkCreateSwapchainKHR(VulkanContext::GetLogicalDevice()->GetNative(), &createInfo, nullptr, &newSwapchain));
		uint32_t imageCount;
		PG_VK_ASSERT(vkGetSwapchainImagesKHR(VulkanContext::GetLogicalDevice()->GetNative(), newSwapchain, &imageCount, nullptr));
		m_SwapchainImages.resize(imageCount);
		PG_VK_ASSERT(vkGetSwapchainImagesKHR(VulkanContext::GetLogicalDevice()->GetNative(), newSwapchain, &imageCount,
			m_SwapchainImages.data()));

		return newSwapchain;
	}

	void Swapchain::CreateRenderPass()
	{
		auto device = VulkanContext::GetLogicalDevice()->GetNative();

		// Attachment description
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = m_SwapChainFormat;
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

		VkSubpassDescription subpassDescription = {};
		subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDescription.colorAttachmentCount = 1;
		subpassDescription.pColorAttachments = &colorAttachmentRef;
		subpassDescription.inputAttachmentCount = 0;
		subpassDescription.pInputAttachments = nullptr;
		subpassDescription.preserveAttachmentCount = 0;
		subpassDescription.pPreserveAttachments = nullptr;
		subpassDescription.pResolveAttachments = nullptr;

		// Render pass 
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpassDescription;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;
		renderPassInfo.pSubpasses;

		PG_VK_ASSERT(vkCreateRenderPass(device,
			&renderPassInfo, nullptr, &m_RenderPass));

		VulkanContext::GetContextResourceFreeQueue().PushBack(RENDERPASS, [device, m_RenderPass = m_RenderPass]()
			{
				vkDestroyRenderPass(device, m_RenderPass, nullptr);
			});
	}

	void Swapchain::CreateImageViews()
	{
		m_SwapchainImageViews.resize(m_SwapchainImages.size());

		for (size_t i = 0; i < m_SwapchainImages.size(); ++i)
		{
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = m_SwapchainImages[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = m_SwapChainFormat;
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			VkImageSubresourceRange image_range = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
			createInfo.subresourceRange = image_range;
			PG_VK_ASSERT(vkCreateImageView(VulkanContext::GetLogicalDevice()->GetNative(), &createInfo, nullptr, &m_SwapchainImageViews[i]));
		}
	}

	void Swapchain::CreateFramebuffers(uint32_t width, uint32_t height)
	{
		m_Framebuffers.resize(m_SwapchainImageViews.size());
		for (size_t i = 0; i < m_SwapchainImageViews.size(); i++) {

			VkImageView attachments[] = {
				m_SwapchainImageViews[i] // Color attachment 
			};

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = m_RenderPass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = width;
			framebufferInfo.height = height;
			framebufferInfo.layers = 1;

			PG_VK_ASSERT(vkCreateFramebuffer(VulkanContext::GetLogicalDevice()->GetNative(), &framebufferInfo, nullptr, &m_Framebuffers[i]));
		}
	}
}