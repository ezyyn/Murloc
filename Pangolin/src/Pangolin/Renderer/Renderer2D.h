#pragma once

#include "Pangolin/Renderer/EditorCamera.h"

namespace PG {

	struct RenderData;

	class Renderer2D {
	public:
		struct Statistics
		{
			uint32_t QuadCount = 0;

			uint32_t GetTotalVertexCount() const { return QuadCount * 4; }
			uint32_t GetTotalIndexCount() const { return QuadCount * 6; }
		};

		Renderer2D() { Init(); }
		~Renderer2D() { Shutdown(); }

		void BeginScene(const EditorCamera& camera);
		void DrawQuad(const glm::mat4& transform, const glm::vec4& color, uint32_t texIndex);
		void EndScene();

		const Statistics& GetStats() const;

		void OnResize(uint32_t width, uint32_t height); 

		// Temp
		void* GetTexture();
		glm::vec2 GetTextureSize();
		// Temp

		template<class ...Args>
		static Ref<Renderer2D> Create(Args&& ... args)
		{
			return CreateRef<Renderer2D>(std::forward<Args>(args)...);
		}
	private:
		void Init();
		void Shutdown();
		void StartBatch();
		void Flush();

		void RecordCommandBuffer();

		RenderData* m_RenderData{ nullptr };
	};
}