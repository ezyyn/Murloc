#pragma once

#include "Pangolin/Renderer/Renderer2D.h"

// Vulkan forward declarations
// GENIUS HACK
#ifdef PG_VULKAN_FORWARD_DECLARATIONS
	#include <vulkan/vulkan.h>
#endif

#if defined(PG_VULKAN_FORWARD_DECLARATIONS) || defined(PG_VULKAN_FUNCTIONS)
	using CommandBufferFunc = std::function<void(VkCommandBuffer mainRenderBuffer, VkFramebuffer mainRenderFramebuffer)>;
#endif

namespace PG {

	struct RendererSettings {
		uint32_t FramesInFlight;
	};

	class RenderManager {
	public:
		// Collection of renderers
		struct Renderers {
			Ref<Renderer2D> Renderer2d;
		};

		static void Init();

		static void Shutdown();

		[[nodiscard]] static const Ref<Renderer2D>& GetRenderer2D() {
			return s_Renderers.Renderer2d;
		}

#if defined(PG_VULKAN_FORWARD_DECLARATIONS) || defined(PG_VULKAN_FUNCTIONS)
		// Retrieves recorded command buffers
		static std::vector<VkCommandBuffer> DrawData();

		// Must always come last if secondary command buffers need to be executes
		static void SubmitPrimary(const std::function<VkCommandBuffer(uint32_t)>& func);

		static void SubmitSecondary(const std::function<VkCommandBuffer(uint32_t, uint32_t)>& func);

		static void SubmitToMain(CommandBufferFunc&& recordedCommandBuffer);

		static void ImmediateSubmit(std::function<void(VkCommandBuffer)>&& function);

		static VkDescriptorSet AllocateImGuiTextureDescriptorSet(VkSampler sampler, VkImageView imageview, VkImageLayout imageLayout);
#endif
		static void OnResize(uint32_t width, uint32_t height);

		static RendererSettings GetSettings() {

			static RendererSettings settings{};
			settings.FramesInFlight = 3; // Triple buffering

			return settings;
		}

	private:
		static inline Renderers s_Renderers;
	};

}
