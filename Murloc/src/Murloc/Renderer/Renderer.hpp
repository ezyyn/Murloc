#pragma once

#include <glm/glm.hpp>

namespace Murloc {

	struct RendererSettings {
		uint32_t FramesInFlight;
	};

	class Renderer {
	public:
		static void Init();
		static void Shutdown();

		[[deprecated]] static void OnResize(uint32_t width, uint32_t height);

		static void BeginScene();
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color);
		static void EndScene();

		static RendererSettings GetSettings()  
		{
			static RendererSettings settings;
			settings.FramesInFlight = 3; // Triple buffering

			return settings;
		}
	private:
		static void StartBatch();
		static void Flush();
	};
}